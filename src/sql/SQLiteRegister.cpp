/*
*   Copyright 2017 Aurélien Milliat <aurelien.milliat@gmail.com>
*   Licensed under the Apache License, Version 2.0 (the "License");
*   you may not use this file except in compliance with the License.
*   You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing,software
*  distributed under the License is distributed on an "AS IS" BASIS,
*WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*/

#include "sql/SQLiteRegister.hpp"
#include <thread>

const std::unordered_map<TableDefinitionBase::ValueDefinition,const std::string> TableDefinitionBase::ValueDefinitionDictionnary=std::unordered_map<TableDefinitionBase::ValueDefinition,const std::string>(
                                                                                                                                {{TableDefinitionBase::ValueDefinition::VD_INTEGER,"INTEGER"},
                                                                                                                                {TableDefinitionBase::ValueDefinition::VD_REAL,"REAL"},
                                                                                                                                {TableDefinitionBase::ValueDefinition::VD_TEXT,"TEXT"},
                                                                                                                                {TableDefinitionBase::ValueDefinition::VD_BLOB,"BLOB"},
                                                                                                                                {TableDefinitionBase::ValueDefinition::VD_NULL,"NULL"}});

const std::unordered_map<TableDefinitionBase::ValueParameter,const std::string> TableDefinitionBase::ValueParameterDictionnary=std::unordered_map<TableDefinitionBase::ValueParameter,const std::string>(
                                                                                                                                {{TableDefinitionBase::ValueParameter::VP_NOT_NULL,"NOT NULL"},
                                                                                                                                {TableDefinitionBase::ValueParameter::VP_PRIMARY_KEY,"PRIMARY KEY"},
																																{TableDefinitionBase::ValueParameter::VP_AUTOINCREMENT,"AUTOINCREMENT"},
																																{TableDefinitionBase::ValueParameter::VP_UNIQUE,"UNIQUE"} });

TableDefinitionBase::TableDefinitionBase(const std::string& name): _Name(name)
{
}

std::string TableDefinitionBase::Name() const
{
	return _Name;
}

SQLiteManager::SQLiteManager( const std::string& dbFilename, const bool& createIfNotExist) 
    : _SQLDB(nullptr), _DBIsCorrect(false), _DBIsCreated(false), _AccumulateQuerries(false)
{
    if (sqlite3_initialize()!=SQLITE_OK)
		Logger::log(SQL_LOG) << "SQLiteManager -> SQLiteManager() : Failed to intialise sqlite3.";
    else
    {
		Logger::log(SQL_LOG) << "SQLiteManager -> SQLiteManager() : sqlite3 initialised.";
        if(!dbFilename.empty())
            _DBIsCorrect=LoadDataBase(dbFilename, createIfNotExist);
    }     
}

SQLiteManager::~SQLiteManager()
{
    sqlite3_close(_SQLDB);
	Logger::log(SQL_LOG) << "SQLiteManager -> ~SQLiteManager() : database closed.";
    sqlite3_shutdown();
	Logger::log(SQL_LOG) << "SQLiteManager -> ~SQLiteManager() : sqlite3 down.";
}

bool SQLiteManager::IsInitialised()
{
    return _DBIsCorrect;
}

bool SQLiteManager::IsCreated()
{
    return _DBIsCreated;
}

void SQLiteManager::SetAccumulation()
{
    _AccumulateQuerries=true;
}

bool SQLiteManager::ReleaseAccumulation()
{
    _AccumulateQuerries=false;
    return Execute();
}
 
bool SQLiteManager::LoadDataBase(const std::string& filename, const bool& createIfNotExist)
{
    if(_SQLDB)
        sqlite3_close(_SQLDB);
    _DBIsCorrect = _DBIsCreated = false;
    if (sqlite3_open_v2(filename.c_str(), &_SQLDB, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK)
    {
        if(!createIfNotExist)
        {
			Logger::log(SQL_LOG) << "SQLiteManager -> LoadDataBase() : Failed to load database: "+filename;
            return false;
        }
        if(sqlite3_open_v2(filename.c_str(), &_SQLDB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr)==SQLITE_OK)
        {
			Logger::log(SQL_LOG) << "SQLiteManager -> LoadDataBase() : database created: "+filename;
            _DBIsCreated = true;
        }
        else
        {
			Logger::log(SQL_LOG) << "SQLiteManager -> LoadDataBase() : Failed to load or create database: "+filename;
            return false;
        }
    }
    else
		Logger::log(SQL_LOG) << "SQLiteManager -> LoadDataBase() : database loaded: "+filename;
    _DBIsCorrect=true;
    return true;
}

bool SQLiteManager::IsTableExist(const std::string& name)
{
    int retVal(-1);
     if(!PrepareAndExecute("SELECT count(type) FROM sqlite_master WHERE name='?001' AND type='table';",{{TableDefinitionBase::ValueDefinition::VD_TEXT,name}},
            &[](void *data, int argc, char **argv, char **azColName)
            {
                int* ret = static_cast<int*>(data);
                if (std::atoi(argv[0])==1)
                    (*ret)=1;
                else
                    (*ret)=0;
                return 0;
            }
            ,&retVal))
        return false;
    while (retVal==-1)
         std::this_thread::sleep_for(std::chrono::microseconds(100));
    return bool(retVal);
}

bool SQLiteManager::PrepareAndExecute(const std::string& querry,const std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& values) const
{
    sqlite3_stmt* pStmt(nullptr);
    int retLite(10);
    if(sqlite3_prepare_v2(_SQLDB,querry.c_str(),querry.size()+1,&pStmt,nullptr)!=SQLITE_OK)
    {
		Logger::log(SQL_LOG) << std::string("SQLiteManager -> PrepareAndExecute() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
        return false;
    }
    for(size_t iterator(0); iterator<values.size(); iterator++)
    {
        switch(values[iterator].first)
        {
            case TableDefinitionBase::ValueDefinition::VD_TEXT:
                if((retLite=sqlite3_bind_text(pStmt,iterator+1,values[iterator].second.c_str(),values[iterator].second.size(),nullptr))!=SQLITE_OK)
                {
                    Logger::log(SQL_LOG) << std::string("SQLiteManager -> PrepareAndExecute() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
                    return false;
                }
                break;
            case TableDefinitionBase::ValueDefinition::VD_BLOB:
                if((retLite=sqlite3_bind_blob(pStmt,iterator+1,values[iterator].second.c_str(),values[iterator].second.size(),nullptr))!=SQLITE_OK)
                {
                    Logger::log(SQL_LOG) << std::string("SQLiteManager -> PrepareAndExecute() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
                    return false;
                }
                break;
            case TableDefinitionBase::ValueDefinition::VD_REAL:
                if((retLite=sqlite3_bind_double(pStmt,iterator+1,std::stod(values[iterator].second)))!=SQLITE_OK)
                {
                    Logger::log(SQL_LOG) << std::string("SQLiteManager -> PrepareAndExecute() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
                    return false;
                }
                break;
            case TableDefinitionBase::ValueDefinition::VD_INTEGER:
                if((retLite=sqlite3_bind_int(pStmt,iterator+1,std::stol(values[iterator].second)))!=SQLITE_OK)
                {
                    Logger::log(SQL_LOG) << std::string("SQLiteManager -> PrepareAndExecute() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
                    return false;
                }
                break;
            case TableDefinitionBase::ValueDefinition::VD_NULL:
                if((retLite=sqlite3_bind_null(pStmt,iterator+1))!=SQLITE_OK)
                {
                    Logger::log(SQL_LOG) << std::string("SQLiteManager -> PrepareAndExecute() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
                    return false;
                }
                break;
        }
    }
    int retValue(SQLITE_BUSY), callbackRet;
    bool boolValue(true);
    while(retValue!=SQLITE_DONE)
    {
        while((retValue=sqlite3_step(pStmt))==SQLITE_BUSY)
            sqlite3_sleep(10);
        switch(retValue)
        {
            case SQLITE_ROW:
            case SQLITE_BUSY:
                continue;
            default: 
                Logger::log(SQL_LOG) << std::string("SQLiteManager -> PrepareAndExecute() : Failed to execute querry: ")+sqlite3_errmsg(_SQLDB);
                boolValue=false;
                retValue=SQLITE_DONE;
            case SQLITE_DONE:
                break;
        }
    }
    sqlite3_finalize(pStmt);
    return boolValue;
}

bool SQLiteManager::Prepare(const std::string & querry,const std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& values)
{
    sqlite3_stmt* pStmt(nullptr);
    int retLite(10);
    if(sqlite3_prepare_v2(_SQLDB,querry.c_str(),querry.size()+1,&pStmt,nullptr)!=SQLITE_OK)
    {
        Logger::log(SQL_LOG) << std::string("SQLiteManager -> Prepare() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
        return false;
    }
    for(size_t iterator(0); iterator<values.size(); iterator++)
    {
        switch(values[iterator].first)
        {
            case TableDefinitionBase::ValueDefinition::VD_TEXT:
                if((retLite=sqlite3_bind_text(pStmt,iterator+1,values[iterator].second.c_str(),values[iterator].second.size(),nullptr))!=SQLITE_OK)
                {
                    Logger::log(SQL_LOG) << std::string("SQLiteManager -> Prepare() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
                    return false;
                }
                break;
            case TableDefinitionBase::ValueDefinition::VD_BLOB:
                if((retLite=sqlite3_bind_blob(pStmt,iterator+1,values[iterator].second.c_str(),values[iterator].second.size(),nullptr))!=SQLITE_OK)
                {
					Logger::log(SQL_LOG) << std::string("SQLiteManager -> Prepare() : Failed to prepare querry: ") + sqlite3_errmsg(_SQLDB);
                    return false;
                }
                break;
            case TableDefinitionBase::ValueDefinition::VD_REAL:
                if((retLite=sqlite3_bind_double(pStmt,iterator+1,std::stod(values[iterator].second)))!=SQLITE_OK)
                {
                    Logger::log(SQL_LOG) << std::string("SQLiteManager -> Prepare() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
                    return false;
                }
                break;
            case TableDefinitionBase::ValueDefinition::VD_INTEGER:
                if((retLite=sqlite3_bind_int(pStmt,iterator+1,std::stol(values[iterator].second)))!=SQLITE_OK)
                {
                    Logger::log(SQL_LOG) << std::string("SQLiteManager -> Prepare() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
                    return false;
                }
                break;
            case TableDefinitionBase::ValueDefinition::VD_NULL:
                if((retLite=sqlite3_bind_null(pStmt,iterator+1))!=SQLITE_OK)
                {
                    Logger::log(SQL_LOG) << std::string("SQLiteManager -> Prepare() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
                    return false;
                }
                break;
        }
    }
    _Querries.push_back(pStmt);
    return true;
}

bool SQLiteManager::Execute()
{ 
    bool boolValue(true);
    for(sqlite3_stmt* pStmt:_Querries)
    {
        int retLite(10);
        int retValue(SQLITE_BUSY), callbackRet;
        while(retValue!=SQLITE_DONE)
        {
            while((retValue=sqlite3_step(pStmt))==SQLITE_BUSY)
                sqlite3_sleep(10);
            switch(retValue)
            {
                case SQLITE_ROW:
                case SQLITE_BUSY:
                    continue;
                default: 
                    Logger::log(SQL_LOG) << std::string("SQLiteManager -> Execute() : Failed to execute querry: ")+sqlite3_errmsg(_SQLDB);
                    boolValue&=false;
                    retValue=SQLITE_DONE;
                case SQLITE_DONE:
                    break;
            }
        }
        sqlite3_finalize(pStmt);   
    }
    _Querries.clear();
    return boolValue;
}

unsigned long  SQLiteManager::GetLastInsertRowID()
{
    return sqlite3_last_insert_rowid(_SQLDB);
}