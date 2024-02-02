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
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*/

#ifndef SQLITEREGISTER_HXX
#define SQLITEREGISTER_HXX

template<typename enumDef>
TableDefinition<enumDef>::TableDefinition() : _Attributes({})
{}

template<typename enumDef>
TableDefinition<enumDef>::TableDefinition(const std::string& name,const std::unordered_map<const std::string,const TableDefinition::ValueDefinition>& definition,const std::unordered_map<enumDef,const std::string>& attributes,const std::unordered_map<enumDef,const std::list<TableDefinition::ValueParameter>>& parameters):
	TableDefinitionBase(name),_Definition(definition),_Attributes(attributes),_Parameters(parameters)
{
    if(_Definition.size()!=_Attributes.size())
        throw std::invalid_argument("TableDefinition -> Table "+name+" definition and attributes number are different.");
}

template<typename enumDef>
TableDefinition<enumDef>::~TableDefinition()
{}

template<typename enumDef>
unsigned int TableDefinition<enumDef>::GetSize() const
{
	return _Definition.size();
}

template<typename enumDef>
std::string TableDefinition<enumDef>::CreateTableQuerry() const
{
    std::string querry("CREATE TABLE \'"+_Name+"\' (");
    for(const auto& iterator:_Attributes)
    {
        querry+="\'"+iterator.second+"\' "+ValueDefinitionDictionnary.at(_Definition.at(iterator.second));
        if(_Parameters.find(iterator.first)!=_Parameters.end())
            for(const TableDefinitionBase::ValueParameter& parameter:_Parameters.at(iterator.first))
                querry+=" "+ValueParameterDictionnary.at(parameter);
        querry+=",";
    }
    querry.pop_back();
    querry+=");";
    return querry;
}

template<typename enumDef>
std::string TableDefinition<enumDef>::Definitions() const
{
    std::string retStr("(");
    retStr.reserve(1024);
    for(const std::pair<enumDef,std::string>& attribute:_Attributes)
        retStr+=attribute.second+(attribute.first==0?"\tPRIMARY KEY":"\t")+ValueDefinitionDictionnary.at(_Definition.at(attribute.second))+",";
    retStr.erase(retStr.size()-1);
    retStr+=")";
    return retStr;
}

template<typename enumDef>
TableDefinitionBase::ValueDefinition TableDefinition<enumDef>::Definition(const enumDef& attribute) const
{
    try
    {
        return _Definition.at(_Attributes.at(attribute));
    }
    catch(...)
    {
        throw std::invalid_argument("TableDefinition -> Definition() : Table "+_Name+" definition for "+std::to_string(attribute)+" invalid.");
    }
}

template<typename enumDef>
std::list<TableDefinitionBase::ValueParameter> TableDefinition<enumDef>::Parameters(const enumDef& attribute) const
{
    try
    {
        return _Parameters.at(_Attributes.at(attribute));
    }
    catch(...)
    {
        throw std::invalid_argument("TableDefinition -> Parameters() : Table "+_Name+" definition for "+std::to_string(attribute)+" invalid.");
    }
}

template<typename enumDef>
std::string TableDefinition<enumDef>::Attributes(const bool& withID) const
{
    std::string retStr("(");
    retStr.reserve(512);
    for(const std::pair<enumDef,const std::string>& attribute:_Attributes)
    {
        if(!withID&&attribute.first==0)
            continue;
        retStr.append(attribute.second+",");
    }
    retStr.erase(retStr.size()-1);
    retStr.append(")");
    return retStr;
}

template<typename enumDef>
std::string TableDefinition<enumDef>::Attribute(const enumDef& attribute) const
{
	return std::string(magic_enum::enum_name(attribute));
}

template<class Callback, class Functor>
bool SQLiteManager::Execute(const std::string& querry, Callback callback, Functor* functor) const
{
    if(!_DBIsCorrect)
        return false;
    char* errorLog(nullptr);
    if(sqlite3_exec(_SQLDB, querry.c_str(), callback, (void *)functor, &errorLog)!=SQLITE_OK)
    {
        Logger::log(SQL_LOG) << std::string("SQLiteManager -> Execute() : Failed to exectue querry: "+querry+"\nError: "+std::string(errorLog));
        sqlite3_free(errorLog);
        return false;
    }
    return true;
}

template<typename Callback, typename Functor>
bool SQLiteManager::PrepareAndExecute(const std::string& querry,const std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& values, Callback* callback, Functor* functor) const
{
    sqlite3_stmt* pStmt(nullptr);
    int retLite(10);
    if((retLite=sqlite3_prepare_v2(_SQLDB,querry.c_str(),querry.size()+1,&pStmt,nullptr))!=SQLITE_OK)
    {
		Logger::log(SQL_LOG) << std::string("SQLiteManager -> PrepareAndExecute() : Failed to prepare querry: ")+sqlite3_errmsg(_SQLDB);
        return false;
    }
    for(size_t iterator(0); iterator<values.size(); iterator++)
    { 
        try
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
        catch(const std::exception& e)
        {
            Logger::log(SQL_LOG) << std::string("SQLiteManager -> PrepareAndExecute() : Failed to parse value : ")+values[iterator].second+" "+e.what();
            return false;
        }
        
    }
    int retValue, callbackRet(1);
    bool boolValue(true),dataProcessed(false);
    while(retValue!=SQLITE_DONE)
    {
        while((retValue=sqlite3_step(pStmt))==SQLITE_BUSY)
            sqlite3_sleep(10);
        if(retValue==SQLITE_ROW && callback!=nullptr)
        {
            int columnCount(sqlite3_column_count(pStmt));
            char** argv=new char*[columnCount];
            char** azColName=new char*[columnCount];
            for(int iterator(0); iterator < columnCount; iterator++)
            {
                argv[iterator]=(char*)sqlite3_column_text(pStmt, iterator);
                azColName[iterator]=(char*)sqlite3_column_name(pStmt, iterator);
            }
            if((callbackRet=(*callback)(functor,columnCount,argv,azColName)))
                Logger::log(SQL_LOG) << "SQLiteManager -> PrepareAndExecute : Callback return code: "+std::to_string(callbackRet)+" stopping process.";
            delete[] argv;
            delete[] azColName;
            if(callbackRet)
               return false;
            dataProcessed=true;
            continue;
        }
        switch(retValue)
        { 
           case SQLITE_DONE:
               if(callback != nullptr &&!dataProcessed)
				   Logger::log(SQL_LOG) << "SQLiteManager -> PrepareAndExecute : Querry has not returned any value.";
               break;
            case SQLITE_ROW:
            case SQLITE_BUSY:
                continue;
            default: 
                Logger::log(SQL_LOG) << std::string("SQLiteManager -> PrepareAndExecute() : Failed to execute querry: ")+sqlite3_errmsg(_SQLDB);

        }
    }
    sqlite3_finalize(pStmt);
    return boolValue;
}
   
template<typename enumDef,typename Callback, class Functor>      
bool SQLiteManager::GetEntryCount(const TableDefinition<enumDef>& table,const enumDef& type, Callback callback, Functor* functor) const
{
    return Execute("SELECT COUNT(DISTINCT "+table.Attribute(type)+") FROM "+ table.Name()+";",callback,functor);
}

template<typename enumDef,class Functor>
bool SQLiteManager::ForwardData(const TableDefinition<enumDef>& table, Functor& functor) const
{
    if(!_DBIsCorrect)
        return false;
    class TempStorage
    {
        public:
            TempStorage(const TableDefinition<enumDef>* const table, Functor& functor):
                _Table(table), _Functor(functor){}
        public:
            const TableDefinition<enumDef>* const _Table;
            Functor& _Functor;
    } storage(&table, functor);
    return Execute("SELECT * FROM "+table.Name()+";",
            [](void *data, int argc, char **argv, char **azColName)
                {
                    for(int it(0);it<argc;it++)
                    {
                        std::string name(azColName[it]);
                        TableDefinitionBase::ValueDefinition  type(static_cast<const TableDefinitionBase*>(static_cast<TempStorage *>(data)->_Table)->Definition(name));
                        static_cast<TempStorage *>(data)->_Functor(name,type,argv[it]);
                    }
                    return 0;
                },&storage);
}
     
template<typename enumDef>
bool SQLiteManager::CreateTable(const TableDefinition<enumDef>& table)
{
    if(Execute(table.CreateTableQuerry(),nullptr))
        Logger::log(SQL_LOG) << "SQLiteManager -> CreateTable() : Table "+table.Name()+" Created.";
    else
    {
        Logger::log(SQL_LOG) << "SQLiteManager -> CreateTable() : Failed to create table "+table.Name();
        return false;
    }
    return true;
}

template<typename enumDef>
bool SQLiteManager::BaseInsert(const std::string& base,const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& attributes)
{
    if(!_DBIsCorrect)
        return false;
    int querry(1);
    std::string sqlQuerry(base+" INTO "+table.Name()+" (");
    sqlQuerry.reserve(512);
    std::string sqlQuerry2(") VALUES (");
    sqlQuerry2.reserve(512);
    std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>> attrs;
    for(const std::pair<unsigned int,std::string>& attr:attributes)
    {
        try
        {
            sqlQuerry+=table.Attribute(attr.first);
            sqlQuerry2+="?"+std::to_string(querry++);
            attrs.push_back({table.Definition(attr.first),attr.second});
        }
        catch(...)
        {
            Logger::log(SQL_LOG) << std::string("SQLiteManager -> BaseInsert() : Failed to process insert on "+table.Name()+" mismatch size between attributes given and table definition.");
			Logger::log(SQL_LOG) << std::string("\t Given : "+std::to_string(attr.first)+" Needed: "+std::to_string(table.GetSize()));
            return false;
        }
        if(querry<(attributes.size()+1))
        {
            sqlQuerry+=",";
            sqlQuerry2+=",";
        }
    }
    sqlQuerry+=sqlQuerry2+");";
    if(_AccumulateQuerries)
        return Prepare(sqlQuerry,attrs);
    return PrepareAndExecute(sqlQuerry,attrs);
}

template<typename enumDef>
bool SQLiteManager::BaseOrderBy(const TableDefinition<enumDef>& table,const std::list<std::pair<long,OrderBy>>& orderBy,std::string& querry)
{
    unsigned long counter(0);
    querry+=" ORDER BY ";
    for(const std::pair<long,OrderBy>& pair:orderBy)
    {
        try
        {
            querry+=table.Attribute(pair.first);
        }
        catch(...)
        {
            Logger::log(SQL_LOG) << std::string("SQLiteManager -> BaseOrderBy() : Failed to process select on "+table.Name()+" mismatch size between attributes given and table definition.");
			Logger::log(SQL_LOG) << std::string("\t Given : "+std::to_string(pair.first)+" Needed: "+std::to_string(table.GetSize()));
            return false;
        }
        querry+=pair.second==OrderBy::OB_ASC?" ASC":" DESC";
        counter++;
        if(counter<(orderBy.size()))
            querry+=",";
    }
    return true;
}

template<typename enumDef>
bool SQLiteManager::Insert(const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& attributes)
{
    return BaseInsert("INSERT",table,attributes);
}

template<typename enumDef>
bool SQLiteManager::InsertIfNotExist(const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& attributes)
{
    return BaseInsert("INSERT OR IGNORE",table,attributes);
}

template<typename enumDef>
bool SQLiteManager::InsertOrReplace(const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& attributes)
{
    return BaseInsert("INSERT OR REPLACE",table,attributes);
}

template<typename enumDef>
bool SQLiteManager::Update(const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& attributes,const std::list<std::list<std::pair<long,std::string>>>& conditions)
{
    if(!_DBIsCorrect)
        return false;
    unsigned long querry(1);
    std::string sqlQuerry("UPDATE "+table.Name()+" SET ");
    sqlQuerry.reserve(512);
    std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>> attrs;
    if(!ProcessList(sqlQuerry,querry,attrs,table,attributes,","))
        return false;
    if(!conditions.empty())
    {
        sqlQuerry+=" WHERE ";
        if(!ProcessList(sqlQuerry,querry,attrs,table,conditions,SQL_NEGATIVE_SEPARATOR,SQL_POSITIVE_SEPARATOR))
            return false;
    }
    sqlQuerry+=";";
    if(_AccumulateQuerries)
        return Prepare(sqlQuerry,attrs);
    return PrepareAndExecute(sqlQuerry,attrs);
}

template<typename enumDef>
bool SQLiteManager::Delete(const TableDefinition<enumDef>& table,const std::list<std::list<std::pair<long,std::string>>>& conditions)
{
    if(!_DBIsCorrect||conditions.empty())
        return false;
    unsigned long querry(1);
    std::string sqlQuerry("DELETE FROM "+table.Name()+" WHERE ");
    sqlQuerry.reserve(512);
    std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>> attrs;
    if(!ProcessList(sqlQuerry,querry,attrs,table,conditions,SQL_NEGATIVE_SEPARATOR,SQL_POSITIVE_SEPARATOR))
        return false;
    sqlQuerry+=";";
    if(_AccumulateQuerries)
        return Prepare(sqlQuerry,attrs);
    return PrepareAndExecute(sqlQuerry,attrs);
}

template<typename enumDef>
bool SQLiteManager::ProcessList(std::string& querry,unsigned long& number,std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& attributes,const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& list,const std::string& separator) const
{
    unsigned int counter(0);
  //  querry+="(";
    for(const std::pair<unsigned int,std::string>& attr:list)
    {
        try
        {
            querry+=table.Attribute(attr.first);
            attributes.push_back({table.Definition(attr.first>0?attr.first:-attr.first),attr.second});
        }
        catch(...)
        {
            Logger::log(SQL_LOG) << std::string("SQLiteManager -> ProcessList() : Failed to process on "+table.Name()+" mismatch size between attributes given and table definition.");
			Logger::log(SQL_LOG) << std::string("\t Given : "+std::to_string(attr.first>0?attr.first:-attr.first)+" Needed: "+std::to_string(table.GetSize()));
            return false;
        }
        counter++;
        querry+="=?"+std::to_string(number++);
        if(counter<list.size())
            querry+=" "+separator+" ";
    }
   // querry+=")";
    return true;
}

template<typename enumDef>
bool SQLiteManager::ProcessList(std::string& querry,unsigned long& number,std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& attributes,const TableDefinition<enumDef>& table,const std::list<std::list<std::pair<long,std::string>>>& list,const std::string& negativeSeparator,const std::string& positiveSeparator) const
{
    for(const std::list<std::pair<long,std::string>>& listt:list)
    {
        unsigned int counter(0);
        /*if(listt.size()>1)
            querry+="(";*/
        for(const std::pair<long,std::string>& attr:listt)
        {
            try
            {
                querry+=table.Attribute(attr.first);
                attributes.push_back({table.Definition(attr.first),attr.second});
            }
            catch(...)
            {
                Logger::log(SQL_LOG) << std::string("SQLiteManager -> ProcessList() : Failed to process on "+table.Name()+" mismatch size between attributes given and table definition.");
				Logger::log(SQL_LOG) << std::string("\t Given : "+std::to_string(attr.first)+" Needed: "+std::to_string(table.GetSize()));
                return false;
            }
            counter++;
            querry+="=?"+std::to_string(number++);
            if(counter<listt.size())
            {
                if(attr.first>0)
                    querry+=" "+positiveSeparator+" ";
                else
                    querry+=" "+negativeSeparator+" ";
            }
        }
       /* if(listt.size()>1)
            querry+=")";*/
    }
    return true;
}

template<typename multiple>
void SQLiteManager::ProcessMultiple(std::string& querry,const std::string& table,const std::string& attribute,const std::string& separator,const multiple& set) const
{
    unsigned long increment(0);
    for(const auto& value:set)
    {
        if(table.empty())
            querry+=attribute+" = "+std::to_string(value);
        else
            querry+=table+"."+attribute+" = "+std::to_string(value);
        increment++;
        if (increment<set.size())
            querry+=separator;
    }
}       

template<typename enumDef>
bool SQLiteManager::BaseSelect(const TableDefinition<enumDef>& table,const std::list<enumDef>& selection,const std::list<std::list<std::pair<long,std::string>>>& conditions,std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& attrs,std::string& sqlQuerry) const
{
    if(!_DBIsCorrect)
        return false;
    unsigned long querry(0);
    unsigned long iterator(0);
    sqlQuerry+="SELECT ";
    for(const unsigned int& select:selection)
    {
        try
        {
            sqlQuerry+=table.Attribute(select);
            querry++;
        }
        catch(...)
        {
            Logger::log(SQL_LOG) << std::string("SQLiteManager -> Select() : Failed to process select on "+table.Name()+" mismatch size between attributes given and table definition.");
			Logger::log(SQL_LOG) << std::string("\t Given : "+std::to_string(select)+" Needed: "+std::to_string(table.GetSize()));
            continue;
        }
        if(querry<selection.size())
            sqlQuerry+=",";
    }
    sqlQuerry+=" FROM "+table.Name();
    if(!conditions.empty())
    {
        querry=1;
        sqlQuerry+=" WHERE ";
        if(!ProcessList(sqlQuerry,querry,attrs,table,conditions,SQL_NEGATIVE_SEPARATOR,SQL_POSITIVE_SEPARATOR))
            return false;
    }
    return true;
}

template<typename enumDef,typename Callback,typename Functor>
bool SQLiteManager::Select(const TableDefinition<enumDef>& table,const std::list<enumDef>& selection,const std::list<std::list<std::pair<long,std::string>>>& conditions,Callback callback,Functor* functor) const
{
    std::string sqlQuerry;
    std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>> attrs;
    if(!BaseSelect(table,selection,conditions,attrs,sqlQuerry))
        return false;
    sqlQuerry+=";";
    return PrepareAndExecute(sqlQuerry,attrs,callback,functor);
}

template<typename enumDef,typename Callback,typename Functor>
bool SQLiteManager::Select(const TableDefinition<enumDef>& table,const std::list<enumDef>& selection,const std::list<std::list<std::pair<long,std::string>>>& conditions,const std::list<std::pair<long,OrderBy>>& orderBy,Callback callback,Functor* functor) const
{
    std::string sqlQuerry;
    std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>> attrs;
    if(!BaseSelect(table,selection,conditions,attrs,sqlQuerry))
        return false;
    if(!BaseOrderBy(table,orderBy,sqlQuerry))
    sqlQuerry+=";";
    return PrepareAndExecute(sqlQuerry,attrs,callback,functor);
}


#endif /* SQLITEREGISTER_HXX */

