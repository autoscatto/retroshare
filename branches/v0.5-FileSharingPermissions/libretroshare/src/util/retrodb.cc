
/*
 * RetroShare : RetroDb functionality
 *
 * Copyright 2012 Christopher Evi-Parker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */

#include <iostream>
#include <sstream>
#include <memory.h>
#include <time.h>
#include <inttypes.h>

#include "retrodb.h"

//#define RETRODB_DEBUG

void free_blob(void* dat){

    char* c = (char*) dat;
    delete[] c;
    dat = NULL;

}

const uint8_t ContentValue::BOOL_TYPE  = 1;
const uint8_t ContentValue::DATA_TYPE = 2;
const uint8_t ContentValue::STRING_TYPE = 3;
const uint8_t ContentValue::DOUBLE_TYPE = 4;
const uint8_t ContentValue::INT32_TYPE = 5;
const uint8_t ContentValue::INT64_TYPE = 6;

const int RetroDb::OPEN_READONLY = SQLITE_OPEN_READONLY;
const int RetroDb::OPEN_READWRITE = SQLITE_OPEN_READWRITE;
const int RetroDb::OPEN_READWRITE_CREATE = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

RetroDb::RetroDb(const std::string &dbPath, int flags) : mDb(NULL) {

    int rc = sqlite3_open_v2(dbPath.c_str(), &mDb, flags, NULL);

    if(rc){
        std::cerr << "Can't open database, Error code: " <<  sqlite3_errmsg(mDb)
                  << std::endl;
        sqlite3_close(mDb);
     }
}


RetroDb::~RetroDb(){

    if(mDb)
        sqlite3_close(mDb);
}

void RetroDb::closeDb(){

    int rc;

    if(mDb)
        rc  = sqlite3_close(mDb);

#ifdef RETRODB_DEBUG
    std::cerr << "RetroDb::closeDb(): Error code on close: " << rc << std::endl;
#endif

}

#define TIME_LIMIT 3

bool RetroDb::execSQL(const std::string &query){

    // prepare statement
    sqlite3_stmt* stm = NULL;

#ifdef RETRODB_DEBUG
    std::cerr << "Query: " << query << std::endl;
#endif

    int rc = sqlite3_prepare_v2(mDb, query.c_str(), query.length(), &stm, NULL);

    // check if there are any errors
    if(rc != SQLITE_OK){
        std::cerr << "RetroDb::execSQL(): Error preparing statement\n";
        std::cerr << "Error code: " <<  sqlite3_errmsg(mDb)
                  << std::endl;
        return false;
    }


    uint32_t delta = 3;
    time_t stamp = time(NULL), now = 0;
    bool timeOut = false, ok = false;

    while(!timeOut){

        rc = sqlite3_step(stm);

        if(rc == SQLITE_DONE){
            ok = true;
            break;
        }

        if(rc != SQLITE_BUSY){
            ok = false;
            break;
        }

        now = time(NULL);
        delta = stamp - now;

        if(delta > TIME_LIMIT){
            ok = false;
            timeOut = true;
        }
        // TODO add sleep so not to waste
        // precious cycles
    }

    if(!ok){

        if(rc == SQLITE_BUSY){
            std::cerr << "RetroDb::execSQL()\n" ;
            std::cerr << "SQL timed out!" << std::endl;
        }else{
            std::cerr << "RetroDb::execSQL(): Error executing statement (code: " << rc << ")\n";
            std::cerr << "Sqlite Error msg: " <<  sqlite3_errmsg(mDb)
                      << std::endl;
        }
    }

    // finalise statement or else db cannot be closed
    sqlite3_finalize(stm);
    return ok;
}

RetroCursor* RetroDb::sqlQuery(const std::string& tableName, const std::list<std::string>& columns,
                               const std::string& selection, const std::string& orderBy){

    if(tableName.empty() || columns.empty()){
        std::cerr << "RetroDb::sqlQuery(): No table or columns given" << std::endl;
        return NULL;
    }

    std::string columnSelection; // the column names to return
    sqlite3_stmt* stmt = NULL;
    std::list<std::string>::const_iterator it = columns.begin();

    for(; it != columns.end(); it++){
        columnSelection += *it;

        it++;
        if(it != columns.end())
            columnSelection += ",";
        it--;
    }

    // construct query
    // SELECT columnSelection FROM tableName WHERE selection
    std::string sqlQuery = "SELECT " + columnSelection + " FROM " +
                           tableName;

    // add selection clause if present
    if(!selection.empty())
        sqlQuery += " WHERE " + selection;


    // add 'order by' clause if present
    if(!orderBy.empty())
        sqlQuery += " ORDER BY " + orderBy + ";";
    else
        sqlQuery += ";";

#ifdef RETRODB_DEBUG
    std::cerr << "RetroDb::sqlQuery(): " << sqlQuery << std::endl;
#endif

    sqlite3_prepare_v2(mDb, sqlQuery.c_str(), sqlQuery.length(), &stmt, NULL);
    return (new RetroCursor(stmt));
}

bool RetroDb::isOpen() const {
    return (mDb==NULL ? false : true);
}

bool RetroDb::sqlInsert(const std::string &table, const std::string& nullColumnHack, const ContentValue &cv){

    std::map<std::string, uint8_t> keyTypeMap;
    cv.getKeyTypeMap(keyTypeMap);
    std::map<std::string, uint8_t>::iterator mit = keyTypeMap.begin();

    // build columns part of insertion
    std::string qColumns = table + "(";

    for(; mit != keyTypeMap.end(); mit++){

        qColumns += mit->first;

        mit++;

        // add comma if more columns left
        if(mit == keyTypeMap.end())
            qColumns += ")"; // close bracket if at end
        else
             qColumns += ",";

        mit--;
    }

    // build values part of insertion
    std::string qValues = "VALUES(";
    std::ostringstream oStrStream;
    uint32_t index = 0;
    std::list<RetroDbBlob> blobL;

    for(mit=keyTypeMap.begin(); mit!=keyTypeMap.end(); mit++){

        uint8_t type = mit->second;
        std::string key = mit->first;

        switch(type){

        case ContentValue::BOOL_TYPE:
            {
                bool value;
                cv.getAsBool(key, value);
                oStrStream << value;
                qValues += oStrStream.str();
                break;
            }
        case ContentValue::DOUBLE_TYPE:
            {
                double value;
                cv.getAsDouble(key, value);
                oStrStream << value;
                qValues += oStrStream.str();
                break;
            }
        case ContentValue::DATA_TYPE:
            {
                char* value;
                uint32_t len;
                cv.getAsData(key, len, value);
                RetroDbBlob b;
                b.data = value;
                b.length = len;
                b.index = ++index;
                blobL.push_back(b);
                qValues += "?"; // parameter
                break;
            }
        case ContentValue::STRING_TYPE:
            {
                std::string value;
                cv.getAsString(key, value);
                qValues += "'" + value +"'";
                break;
            }
        case ContentValue::INT32_TYPE:
            {
                int32_t value;
                cv.getAsInt32(key, value);
                oStrStream << value;
                qValues += oStrStream.str();
                break;
            }
        case ContentValue::INT64_TYPE:
            {
                int64_t value;
                cv.getAsInt64(key, value);
                oStrStream << value;
                qValues += oStrStream.str();
                break;
            }
        }

        mit++;
        if(mit != keyTypeMap.end()){ // add comma if more columns left
            qValues += ",";
        }
        else{ // at end close brackets
             qValues += ");";
         }
        mit--;

        // clear stream strings
        oStrStream.str("");
    }


    // complete insertion query
    std::string sqlQuery = "INSERT INTO " + qColumns + " " + qValues;

#ifdef RETRODB_DEBUG
    std::cerr << "RetroDb::sqlInsert(): " << sqlQuery << std::endl;
#endif

    // execute query
    execSQL_bind_blobs(sqlQuery, blobL);
    return true;
}

bool RetroDb::execSQL_bind_blobs(const std::string &query, std::list<RetroDbBlob> &blobs){

    // prepare statement
    sqlite3_stmt* stm = NULL;

#ifdef RETRODB_DEBUG
    std::cerr << "Query: " << query << std::endl;
#endif

    int rc = sqlite3_prepare_v2(mDb, query.c_str(), query.length(), &stm, NULL);

    // check if there are any errors
    if(rc != SQLITE_OK){
        std::cerr << "RetroDb::execSQL(): Error preparing statement\n";
        std::cerr << "Error code: " <<  sqlite3_errmsg(mDb)
                  << std::endl;
        return false;
    }

    std::list<RetroDbBlob>::iterator lit = blobs.begin();

    for(; lit != blobs.end(); lit++){
        const RetroDbBlob& b = *lit;
        sqlite3_bind_blob(stm, b.index, b.data, b.length, NULL);
    }

    uint32_t delta = 3;
    time_t stamp = time(NULL), now = 0;
    bool timeOut = false, ok = false;

    while(!timeOut){

        rc = sqlite3_step(stm);

        if(rc == SQLITE_DONE){
            ok = true;
            break;
        }

        if(rc != SQLITE_BUSY){
            ok = false;
            break;
        }

        now = time(NULL);
        delta = stamp - now;

        if(delta > TIME_LIMIT){
            ok = false;
            timeOut = true;
        }
        // TODO add sleep so not to waste
        // precious cycles
    }

    if(!ok){

        if(rc == SQLITE_BUSY){
            std::cerr << "RetroDb::execSQL()\n" ;
            std::cerr << "SQL timed out!" << std::endl;
        }else{
            std::cerr << "RetroDb::execSQL(): Error executing statement (code: " << rc << ")\n";
            std::cerr << "Sqlite Error msg: " <<  sqlite3_errmsg(mDb)
                      << std::endl;
        }
    }

    // finalise statement or else db cannot be closed
    sqlite3_finalize(stm);
    return ok;
}

bool RetroDb::sqlDelete(const std::string &tableName, const std::string &whereClause, const std::string &whereArgs){

    std::string sqlQuery = "DELETE FROM " + tableName;

    if(!whereClause.empty()){
        sqlQuery += " WHERE " + whereClause + ";";
    }else
        sqlQuery += ";";

    return execSQL(sqlQuery);
}


bool RetroDb::sqlUpdate(const std::string &tableName, std::string whereClause, const ContentValue& cv){

    std::string sqlQuery = "UPDATE " + tableName + " SET ";


    std::map<std::string, uint8_t> keyTypeMap;
    std::map<std::string, uint8_t>::iterator mit;
    cv.getKeyTypeMap(keyTypeMap);

    // build SET part of update
    std::string qValues = "";
    std::ostringstream oStrStream;

    for(mit=keyTypeMap.begin(); mit!=keyTypeMap.end(); mit++){

        uint8_t type = mit->second;
        std::string key = mit->first;

        switch(type){

        case ContentValue::BOOL_TYPE:
            {
                bool value;
                cv.getAsBool(key, value);
                oStrStream << value;
                qValues += key + "='" + oStrStream.str();
                break;
            }
        case ContentValue::DOUBLE_TYPE:
            {
                double value;
                cv.getAsDouble(key, value);
                oStrStream << value;
                qValues += key + "='" + oStrStream.str();
                break;
            }
        case ContentValue::DATA_TYPE:
            {
                char* value;
                uint32_t len;
                cv.getAsData(key, len, value);
                oStrStream.write(value, len);
                qValues += key + "='" + oStrStream.str() + "' ";
                break;
            }
        case ContentValue::STRING_TYPE:
            {
                std::string value;
                cv.getAsString(key, value);
                qValues += key + "='" + value + "' ";
                break;
            }
        case ContentValue::INT32_TYPE:
            {
                int32_t value;
                cv.getAsInt32(key, value);
                oStrStream << value;
                qValues += key + "='" + oStrStream.str() + "' ";
                break;
            }
        case ContentValue::INT64_TYPE:
            {
                int64_t value;
                cv.getAsInt64(key, value);
                oStrStream << value;
                qValues += key + "='" + oStrStream.str() + "' ";
                break;
            }
        }

        mit++;
        if(mit != keyTypeMap.end()){ // add comma if more columns left
            qValues += ",";
        }
        mit--;

        // clear stream strings
        oStrStream.str("");
    }

    if(qValues.empty())
        return false;
    else
        sqlQuery += qValues;

    // complete update
    if(!whereClause.empty()){
        sqlQuery += " WHERE " +  whereClause + ";";
    }
    else{
        sqlQuery += ";";
    }

    // execute query
    return execSQL(sqlQuery);

}





/********************** RetroCursor ************************/

RetroCursor::RetroCursor(sqlite3_stmt *stmt)
    : mStmt(NULL), mCount(0), mPosCounter(0) {

     open(stmt);
}

RetroCursor::~RetroCursor(){

    // finalise statement
    if(mStmt){
        sqlite3_finalize(mStmt);
    }
}

bool RetroCursor::moveToFirst(){

#ifdef RETRODB_DEBUG
    std::cerr << "RetroCursor::moveToFirst()\n";
#endif

    if(!isOpen())
        return false;

    // reset statement
    int rc = sqlite3_reset(mStmt);

    if(rc != SQLITE_OK){

#ifdef RETRODB_DEBUG
    std::cerr << "Error code: " << rc << std::endl;
#endif

        return false;
    }

    rc = sqlite3_step(mStmt);

    if(rc == SQLITE_ROW){
        mPosCounter = 0;
        return true;
    }

#ifdef RETRODB_DEBUG
    std::cerr << "Error code: " << rc << std::endl;
#endif

    return false;
}

bool RetroCursor::moveToLast(){

#ifdef RETRODB_DEBUG
    std::cerr << "RetroCursor::moveToLast()\n";
#endif

    if(!isOpen())
        return -1;

    // go to begining
    int rc = sqlite3_reset(mStmt);

    if(rc != SQLITE_OK)
        return false;

    rc = sqlite3_step(mStmt);

    while(rc == SQLITE_ROW){
        rc = sqlite3_step(mStmt);
    }

    if(rc != SQLITE_DONE){
        std::cerr << "Error executing statement (code: " << rc << ")\n"
                  << std::endl;
        return false;

    }else{
        mPosCounter = mCount;
        return true;
    }
}

int RetroCursor::getResultCount() const {

    if(isOpen())
        return mCount;
    else
        return -1;
}

bool RetroCursor::isOpen() const {
    return !(mStmt == NULL);
}

bool RetroCursor::close(){

    if(!isOpen())
        return false;


    int rc = sqlite3_finalize(mStmt);
    mStmt = NULL;
    mPosCounter = 0;
    mCount = 0;

    return (rc == SQLITE_OK);
}

bool RetroCursor::open(sqlite3_stmt *stm){

#ifdef RETRODB_DEBUG
    std::cerr << "RetroCursor::open() \n";
#endif

    if(isOpen())
        close();

    mStmt = stm;

    // ensure statement is valid
    int rc = sqlite3_reset(mStmt);

    if(rc == SQLITE_OK){

        while((rc = sqlite3_step(mStmt)) == SQLITE_ROW)
            mCount++;

        sqlite3_reset(mStmt);

        return true;
    }
    else{
        std::cerr << "Error Opening cursor (code: " << rc << ")\n";
        close();
        return false;
    }

}

bool RetroCursor::moveToNext(){

#ifdef RETRODB_DEBUG
    std::cerr << "RetroCursor::moveToNext()\n";
#endif

    if(!isOpen())
        return false;

    int rc = sqlite3_step(mStmt);

    if(rc == SQLITE_ROW){
        mPosCounter++;
        return true;

    }else if(rc == SQLITE_DONE){ // no more results
        return false;
    }
    else if(rc == SQLITE_BUSY){ // should not enter here
        std::cerr << "RetroDb::moveToNext()\n" ;
        std::cerr << "Busy!, possible multiple accesses to Db" << std::endl
                  << "serious error";

        return false;

    }else{
        std::cerr << "Error executing statement (code: " << rc << ")\n";
        return false;
    }
}


int32_t RetroCursor::getPosition() const {

    if(isOpen())
        return mPosCounter;
    else
        return -1;
}


int32_t RetroCursor::getInt32(int columnIndex){
    return sqlite3_column_int(mStmt, columnIndex);
}

int64_t RetroCursor::getInt64(int columnIndex){
    return sqlite3_column_int64(mStmt, columnIndex);
}

bool RetroCursor::getBool(int columnIndex){
    return sqlite3_column_int(mStmt, columnIndex);
}

double RetroCursor::getDouble(int columnIndex){
    return sqlite3_column_double(mStmt, columnIndex);
}

void RetroCursor::getString(int columnIndex, std::string &str){
    char* raw_str = (char*)sqlite3_column_text(mStmt, columnIndex);
    if(raw_str != NULL)
        str.assign(raw_str);
}

const void* RetroCursor::getData(int columnIndex, uint32_t &datSize){

    const void* val = sqlite3_column_blob(mStmt, columnIndex);
    datSize = sqlite3_column_bytes(mStmt, columnIndex);

    return val;
}




/**************** content value implementation ******************/

typedef std::pair<std::string, uint8_t> KeyTypePair;

ContentValue::ContentValue(){

}

ContentValue::~ContentValue(){
    // release resources held in data
    clearData();
}

ContentValue::ContentValue(ContentValue &from){

    std::map<std::string, uint8_t> keyTypeMap;
    from.getKeyTypeMap(keyTypeMap);
    std::map<std::string, uint8_t>::const_iterator cit =
            keyTypeMap.begin();

    uint8_t type = 0;
    std::string currKey;
    std::string val = "";
    char *src = NULL;
    uint32_t data_len = 0;

    for(; cit != keyTypeMap.end(); cit++){

        type = cit->second;
        currKey = cit->first;

        switch(type){

        case INT32_TYPE:
            {
                int32_t value;
                from.getAsInt32(currKey, value);
                put(currKey, value);
                break;
            }
        case INT64_TYPE:
            {
                int64_t value;
                from.getAsInt64(currKey, value);
                put(currKey, value);
                break;
            }
        case STRING_TYPE:
            {
                from.getAsString(currKey, val);
                put(currKey, val);
                break;
            }
        case BOOL_TYPE:
            {
                bool value;
                from.getAsBool(currKey, value);
                put(currKey, value);
                break;
            }
        case DATA_TYPE:
            {
                from.getAsData(currKey, data_len, src);
                put(currKey, data_len, src);
                break;
            }
        case DOUBLE_TYPE:
            double value;
            from.getAsDouble(currKey, value);
            put(currKey, value);
            break;
        default:
            std::cerr << "ContentValue::ContentValue(ContentValue &from):"
                    << "Error! Unrecognised data type!" << std::endl;
        }
    }
}

void ContentValue::put(const std::string &key, bool value){

    if(mKvSet.find(key) != mKvSet.end())
        removeKeyValue(key);

    mKvSet.insert(KeyTypePair(key, BOOL_TYPE));
    mKvBool.insert(std::pair<std::string, bool>(key, value));
}

void ContentValue::put(const std::string &key, const std::string &value){

    if(mKvSet.find(key) != mKvSet.end())
        removeKeyValue(key);

    mKvSet.insert(KeyTypePair(key, STRING_TYPE));
    mKvString.insert(std::pair<std::string, std::string>(key, value));
}

void ContentValue::put(const std::string &key, double value){

    if(mKvSet.find(key) != mKvSet.end())
        removeKeyValue(key);

    mKvSet.insert(KeyTypePair(key,DOUBLE_TYPE));
    mKvDouble.insert(std::pair<std::string, double>(key, value));
}

void ContentValue::put(const std::string &key, int32_t value){

    if(mKvSet.find(key) != mKvSet.end())
        removeKeyValue(key);

    mKvSet.insert(KeyTypePair(key, INT32_TYPE));
    mKvInt32.insert(std::pair<std::string, int32_t>(key, value));
}

void ContentValue::put(const std::string &key, int64_t value){

    if(mKvSet.find(key) != mKvSet.end())
        removeKeyValue(key);

    mKvSet.insert(KeyTypePair(key, INT64_TYPE));
    mKvInt64.insert(std::pair<std::string, int64_t>(key, value));
}

void ContentValue::put(const std::string &key, uint32_t len, const char* value){


    // release memory from old key value if key
    // exists
    if(mKvSet.find(key) != mKvSet.end()) {
        removeKeyValue(key);
    }

    mKvSet.insert(KeyTypePair(key, DATA_TYPE));
    char* dest = NULL;

    // len is zero then just put a NULL entry
    if(len != 0){
        dest  = new char[len];
        memcpy(dest, value, len);
    }

    mKvData.insert(std::pair<std::string, std::pair<uint32_t, char*> >
                   (key, std::pair<uint32_t, char*>(len, dest)));
}

bool ContentValue::getAsBool(const std::string &key, bool& value) const{

    std::map<std::string, bool>::const_iterator it;
    if((it = mKvBool.find(key)) == mKvBool.end())
        return false;

  value = it->second;
  return true;
}

bool ContentValue::getAsInt32(const std::string &key, int32_t& value) const{

    std::map<std::string, int32_t>::const_iterator it;
    if((it = mKvInt32.find(key)) == mKvInt32.end())
        return false;

    value = it->second;
    return true;
}

bool ContentValue::getAsInt64(const std::string &key, int64_t& value) const{

    std::map<std::string, int64_t>::const_iterator it;
    if((it = mKvInt64.find(key)) == mKvInt64.end())
        return false;

    value = it->second;
    return true;
}

bool ContentValue::getAsString(const std::string &key, std::string &value) const{

    std::map<std::string, std::string>::const_iterator it;
    if((it = mKvString.find(key)) == mKvString.end())
        return false;

    value = it->second;
    return true;
}

bool ContentValue::getAsData(const std::string& key, uint32_t &len, char*& value) const{

    std::map<std::string, std::pair<uint32_t, char*> >::const_iterator it;
    if((it = mKvData.find(key)) == mKvData.end())
        return false;

    const std::pair<uint32_t, char*> &kvRef = it->second;

    len = kvRef.first;
    value = kvRef.second;
    return true;
}

bool ContentValue::getAsDouble(const std::string &key, double& value) const{

    std::map<std::string, double>::const_iterator it;
    if((it = mKvDouble.find(key)) == mKvDouble.end())
        return false;

    value = it->second;
    return true;
}

bool ContentValue::removeKeyValue(const std::string &key){

    std::map<std::string, uint8_t>::iterator mit;

    if((mit = mKvSet.find(key)) == mKvSet.end())
        return false;

    if(mit->second == BOOL_TYPE)
        mKvBool.erase(key);

    if(mit->second == INT64_TYPE)
        mKvInt64.erase(key);

    if(mit->second == DATA_TYPE){
        delete[] (mKvData[key].second);
        mKvData.erase(key);
    }

    if(mit->second == DOUBLE_TYPE)
        mKvDouble.erase(key);

    if(mit->second == STRING_TYPE)
        mKvString.erase(key);

    if(mit->second == INT32_TYPE)
        mKvInt32.erase(key);


    mKvSet.erase(key);
    return true;
}


void ContentValue::getKeyTypeMap(std::map<std::string, uint8_t> &keySet) const {
    keySet = mKvSet;
}

void ContentValue::clear(){
    mKvSet.clear();
    mKvBool.clear();
    mKvDouble.clear();
    mKvString.clear();
    mKvInt32.clear();
    mKvInt64.clear();
    clearData();
}

void ContentValue::clearData(){

    std::map<std::string, std::pair<uint32_t, char*> >::iterator
            mit = mKvData.begin();

    for(; mit != mKvData.end(); mit++){

        if(mit->second.first != 0)
            delete[] (mit->second.second);
    }

    mKvData.clear();
}



