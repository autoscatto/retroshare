/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (c) 2010, Thomas Kister
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include <QTextBrowser>
#include "HandleRichText.h"
#include "gui/RetroShareLink.h"

#include <iostream>

namespace RsHtml {

EmbedInHtmlImg defEmbedImg;

void EmbedInHtmlImg::InitFromAwkwardHash(const QHash< QString, QString >& hash)
{
	QString newRE;
	for(QHash<QString,QString>::const_iterator it = hash.begin(); it != hash.end(); ++it)
		foreach(QString smile, it.key().split("|")) {
			if (smile.isEmpty()) {
				continue;
			}
			smileys.insert(smile, it.value());
			newRE += "(" + QRegExp::escape(smile) + ")|";
		}
	newRE.chop(1);	// remove last |
	myRE.setPattern(newRE);
}

/**
 * Parses a DOM tree and replaces text by HTML tags.
 * The tree is traversed depth-first, but only through children of Element type
 * nodes. Any other kind of node is terminal.
 *
 * If the node is of type Text, its data is checked against the user-provided
 * regular expression. If there is a match, the text is cut in three parts: the
 * preceding part that will be inserted before, the part to be replaced, and the
 * following part which will be itself checked against the regular expression.
 *
 * The part to be replaced is sent to a user-provided functor that will create
 * the necessary embedding and return a new Element node to be inserted.
 *
 * @param[in] doc The whole DOM tree, necessary to create new nodes
 * @param[in,out] currentElement The current node (which is of type Element)
 * @param[in] embedInfos The regular expression and the type of embedding to use
 */
static void embedHtml(QDomDocument& doc, QDomElement& currentElement, EmbedInHtml& embedInfos)
{
	if(embedInfos.myRE.pattern().length() == 0)	// we'll get stuck with an empty regexp
		return;

	QDomNodeList children = currentElement.childNodes();
	for(uint index = 0; index < children.length(); index++) {
		QDomNode node = children.item(index);
		if(node.isElement()) {
			// child is an element, we skip it if it's an <a> tag
			QDomElement element = node.toElement();
			if(element.tagName().toLower() == "head") {
				// skip it
			} else if (element.tagName().toLower() == "a") {
				// skip it, but add title if not available
				if (element.attribute("title").isEmpty()) {
					RetroShareLink link(element.attribute("href"));
					QString title = link.title();
					if (!title.isEmpty()) {
						element.setAttribute("title", title);
					}
				}
			} else {
				embedHtml(doc, element, embedInfos);
			}
		}
		else if(node.isText()) {
			// child is a text, we parse it
			QString tempText = node.toText().data();
			if(embedInfos.myRE.indexIn(tempText) == -1)
				continue;

			// there is at least one link inside, we start replacing
			int currentPos = 0;
			int nextPos = 0;
			while((nextPos = embedInfos.myRE.indexIn(tempText, currentPos)) != -1) {
				// if nextPos == 0 it means the text begins by a link
				if(nextPos > 0) {
					QDomText textPart = doc.createTextNode(tempText.mid(currentPos, nextPos - currentPos));
					currentElement.insertBefore(textPart, node);
					index++;
				}

				// inserted tag
				QDomElement insertedTag;
				switch(embedInfos.myType) {
					case Ahref:
							{
								insertedTag = doc.createElement("a");
								insertedTag.setAttribute("href", embedInfos.myRE.cap(0));

								RetroShareLink link(embedInfos.myRE.cap(0));
								QString title = link.title();
								if (!title.isEmpty()) {
									insertedTag.setAttribute("title", title);
								}

								insertedTag.appendChild(doc.createTextNode(embedInfos.myRE.cap(0)));
							}
							break;
					case Img:
							{
								insertedTag = doc.createElement("img");
								const EmbedInHtmlImg& embedImg = static_cast<const EmbedInHtmlImg&>(embedInfos);
								insertedTag.setAttribute("src", embedImg.smileys[embedInfos.myRE.cap(0)]);
							}
							break;
				}
				currentElement.insertBefore(insertedTag, node);

				currentPos = nextPos + embedInfos.myRE.matchedLength();
				index++;
			}

			// text after the last link, only if there's one, don't touch the index
			// otherwise decrement the index because we're going to remove node
			if(currentPos < tempText.length()) {
				QDomText textPart = doc.createTextNode(tempText.mid(currentPos));
				currentElement.insertBefore(textPart, node);
			}
			else
				index--;

			currentElement.removeChild(node);
		}
	}
}

QString formatText(const QString &text, unsigned int flag)
{
	if (flag == 0 || text.isEmpty()) {
		// nothing to do
		return text;
	}

	QDomDocument doc;
	if (doc.setContent(text) == false) {
		// convert text with QTextBrowser
		QTextBrowser textBrowser;
		textBrowser.setText(text);
		doc.setContent(textBrowser.toHtml());
	}

	QDomElement body = doc.documentElement();
	if (flag & RSHTML_FORMATTEXT_EMBED_SMILEYS) {
		embedHtml(doc, body, defEmbedImg);
	}
	if (flag & RSHTML_FORMATTEXT_EMBED_LINKS) {
		EmbedInHtmlAhref defEmbedAhref;
		embedHtml(doc, body, defEmbedAhref);
	}

	QString formattedText = doc.toString(-1);  // -1 removes any annoying carriage return misinterpreted by QTextEdit
	optimizeHtml(formattedText);

	return formattedText;
}

static void findElements(QDomDocument& doc, QDomElement& currentElement, const QString& nodeName, const QString& nodeAttribute, QStringList &elements)
{
	if(nodeName.isEmpty()) {
		return;
	}

	QDomNodeList children = currentElement.childNodes();
	for (uint index = 0; index < children.length(); index++) {
		QDomNode node = children.item(index);
		if (node.isElement()) {
			QDomElement element = node.toElement();
			if (QString::compare(element.tagName(), nodeName, Qt::CaseInsensitive) == 0) {
				if (nodeAttribute.isEmpty()) {
					// use text
					elements.append(element.text());
				} else {
					QString attribute = element.attribute(nodeAttribute);
					if (attribute.isEmpty() == false) {
						elements.append(attribute);
					}
				}
				continue;
			}
			findElements(doc, element, nodeName, nodeAttribute, elements);
		}
	}
}

bool findAnchors(const QString &text, QStringList& urls)
{
	QDomDocument doc;
	if (doc.setContent(text) == false) {
		return false;
	}

	QDomElement body = doc.documentElement();
	findElements(doc, body, "a", "href", urls);

	return true;
}

static void removeElement(QDomElement& parentElement, QDomElement& element)
{
	QDomNodeList children = element.childNodes();
	while (children.length() > 0) {
		QDomNode childElement = element.removeChild(children.item(children.length() - 1));
		parentElement.insertAfter(childElement, element);
	}
	parentElement.removeChild(element);
}

static void optimizeHtml(QDomDocument& doc, QDomElement& currentElement)
{
	if (currentElement.tagName().toLower() == "html") {
		// change <html> to <span>
		currentElement.setTagName("span");
	}

	QDomNode styleNode;
	QDomAttr styleAttr;
	bool addBR = false;

	QDomNodeList children = currentElement.childNodes();
	for (uint index = 0; index < children.length(); ) {
		QDomNode node = children.item(index);

		// compress style attribute
		styleNode = node.attributes().namedItem("style");
		if (styleNode.isAttr()) {
			styleAttr = styleNode.toAttr();
			QString value = styleAttr.value().simplified();
			value.replace("margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;", "margin:0px 0px 0px 0px;");
			value.replace("; ", ";");
			styleAttr.setValue(value);
		}

		if (node.isElement()) {
			QDomElement element = node.toElement();

			// not <p>
			if (addBR && element.tagName().toLower() != "p") {
				// add <br> after a removed <p> but not before a <p>
				QDomElement elementBr = doc.createElement("br");
				currentElement.insertBefore(elementBr, element);
				addBR = false;
				++index;
			}

			// <body>
			if (element.tagName().toLower() == "body") {
				if (element.attributes().length() == 0) {
					// remove <body> without attributes
					removeElement(currentElement, element);
					// no ++index;
					continue;
				}
				// change <body> to <span>
				element.setTagName("span");
			}

			// <head>
			if (element.tagName().toLower() == "head") {
				// remove <head>
				currentElement.removeChild(node);
				// no ++index;
				continue;
			}

			// iterate children
			optimizeHtml(doc, element);

			// <p>
			if (element.tagName().toLower() == "p") {
				// <p style="...">
				//styleNode = element.attributes().namedItem("style");
				if (element.attributes().size() == 1 && styleNode.isAttr()) {
					QString value = styleAttr.toAttr().value().simplified();
					if (value == "margin:0px 0px 0px 0px;-qt-block-indent:0;text-indent:0px;" ||
						value.startsWith("-qt-paragraph-type:empty;margin:0px 0px 0px 0px;-qt-block-indent:0;text-indent:0px;")) {

						if (addBR) {
							// add <br> after a removed <p> before a removed <p>
							QDomElement elementBr = doc.createElement("br");
							currentElement.insertBefore(elementBr, element);
							++index;
						}
						// remove Qt standard <p> or empty <p>
						index += element.childNodes().length();
						removeElement(currentElement, element);
						addBR = true;
						continue;
					 }
				}
				addBR = false;
			}
		}
		++index;
	}
}

void optimizeHtml(QTextEdit *textEdit, QString &text)
{
	if (textEdit->toHtml() == QTextDocument(textEdit->toPlainText()).toHtml()) {
		text = textEdit->toPlainText();
		std::cerr << "Optimized text to " << text.length() << " bytes , instead of " << textEdit->toHtml().length() << std::endl;
		return;
	}

	text = textEdit->toHtml();

	optimizeHtml(text);
}

void optimizeHtml(QString &text)
{
	int originalLength = text.length();

	// remove doctype
	text.remove(QRegExp("<!DOCTYPE[^>]*>"));

	QDomDocument doc;
	if (doc.setContent(text) == false) {
		return;
	}

	QDomElement body = doc.documentElement();
	optimizeHtml(doc, body);
	text = doc.toString(-1);

	std::cerr << "Optimized text to " << text.length() << " bytes , instead of " << originalLength << std::endl;
}

QString toHtml(QString text, bool realHtml)
{
	// replace "\n" from the optimized html with "<br>"
	text.replace("\n", "<br>");
	if (!realHtml) {
		return text;
	}

	QTextDocument doc;
	doc.setHtml(text);
	return doc.toHtml();
}

} // namespace RsHtml
