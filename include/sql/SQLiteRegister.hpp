/*
*   Copyright 2017 AurÃ©lien Milliat <aurelien.milliat@gmail.com>
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

#ifndef SQLITEREGISTER_HPP
#define SQLITEREGISTER_HPP

#include <string>
#include <map>
#include <deque>
#include <set>
#include <list>
#include "magic_enum/magic_enum.hpp"
#include "utils/Logger.h"

extern "C"
{
#include "sqlite/sqlite3.h"
}

#define SQL_LOG LoggerMode::SQLog

#ifndef SQL_POSITIVE_SEPARATOR
#define SQL_POSITIVE_SEPARATOR " AND "
#endif 

#ifndef SQL_NEGATIVE_SEPARATOR
#define SQL_NEGATIVE_SEPARATOR " OR "
#endif 

class TableDefinitionBase
{
    public:
        enum class ValueDefinition{VD_NULL,VD_INTEGER,VD_TEXT,VD_REAL,VD_BLOB};
        enum class ValueParameter{VP_NOT_NULL,VP_PRIMARY_KEY,VP_AUTOINCREMENT,VP_UNIQUE};
        static const std::unordered_map<ValueDefinition,const std::string> ValueDefinitionDictionnary;
        static const std::unordered_map<ValueParameter,const std::string> ValueParameterDictionnary;
	public:
			TableDefinitionBase(const std::string& name);
			std::string Name() const;
	protected:
		std::string _Name;
};

template<typename enumDef>
class TableDefinition : public TableDefinitionBase
{
    public:
        enumDef Elements;
    public:
        TableDefinition();
        TableDefinition(const std::string& name, const std::unordered_map<const std::string,const ValueDefinition>& definition, const std::unordered_map<enumDef,const std::string>& attributes,const std::unordered_map<enumDef,const std::list<TableDefinitionBase::ValueParameter>>& parameters={});
        ~TableDefinition();

        std::string CreateTableQuerry() const;
        std::string Definitions() const;
        ValueDefinition Definition(const enumDef& attribute) const;
        std::string Attributes(const bool& withID=true) const;
        std::string Attribute(const enumDef& attribute) const;
        std::list<ValueParameter> Parameters(const enumDef& attribute) const;
		unsigned int GetSize() const;

    private:
		const std::unordered_map<enumDef, const ValueDefinition> _Definition;
        const std::unordered_map<enumDef, const std::list<TableDefinitionBase::ValueParameter>>   _Parameters;
};

//https://stackoverflow.com/questions/4678237/how-can-i-password-protect-my-sqlite-db-in-c-is-it-possible-to-partition-the-sq/40763590

class SQLiteManager 
{
    public:
        enum OrderBy{OB_ASC,OB_DESC};

    public:
        SQLiteManager(const std::string& dbFilename="", const bool& createIfNotExist=false);
        ~SQLiteManager();
        
        bool IsInitialised();
        bool IsCreated();

        void SetAccumulation();
        bool ReleaseAccumulation();

    protected:
        bool LoadDataBase(const std::string& filename,const bool& createIfNotExist);
        bool IsTableExist(const std::string& name);

        template<typename multiple>
        void ProcessMultiple(std::string& querry,const std::string& table,const std::string& attribute,const std::string& separator,const multiple& set) const;
        
        virtual unsigned long GetLastInsertRowID();

        template<typename enumDef>
        bool CreateTable(const TableDefinition<enumDef>& table);
        
        virtual void CreateTables()=0;

        template<typename enumDef, typename Callback=int(void*, int , char **, char **),class Functor=void*>
        bool GetEntryCount(const TableDefinition<enumDef>& table,const enumDef& type, Callback callback, Functor* functor=nullptr) const;
        
        template<typename enumDef, class Functor>
        bool ForwardData(const TableDefinition<enumDef>& table,Functor& functor) const;

        template<class Callback, class Functor=void*>
        bool Execute(const std::string& querry,Callback callback,Functor* functor=nullptr) const;

        template<typename Callback=int(void*, int , char **, char **), typename Functor=void*>
        bool PrepareAndExecute(const std::string& querry,const std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& values,Callback* callback,Functor* functor=nullptr) const;

        bool PrepareAndExecute(const std::string& querry,const std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& values) const;

        bool Prepare(const std::string& querry,const std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& values);

        bool Execute();

        template<typename enumDef>
        bool BaseInsert(const std::string& base,const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& attributes);

        template<typename enumDef>
        bool BaseSelect(const TableDefinition<enumDef>& table,const std::list<enumDef>& selection,const std::list<std::list<std::pair<long,std::string>>>& conditions,std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& attrs,std::string& sqlQuerry) const;

        template<typename enumDef>
        bool BaseOrderBy(const TableDefinition<enumDef>& table,const std::list<std::pair<long,OrderBy>>& orderBy, std::string& querry);

        template<typename enumDef>
        bool Insert(const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& attributes);

        template<typename enumDef>
        bool InsertIfNotExist(const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& attributes);
        
        template<typename enumDef>
        bool InsertOrReplace(const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& attributes);

        template<typename enumDef>
        bool Update(const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& attributes, const std::list<std::list<std::pair<long, std::string>>>& conditions);

        template<typename enumDef, typename Callback=int(void*, int , char **, char **), typename Functor=void*>
        bool Select(const TableDefinition<enumDef>& table,const std::list<enumDef>& selection,const std::list<std::list<std::pair<long,std::string>>>& conditions,Callback callback,Functor* functor) const;

        template<typename enumDef,typename Callback=int(void*,int,char **,char **),typename Functor=void*>
        bool Select(const TableDefinition<enumDef>& table,const std::list<enumDef>& selection,const std::list<std::list<std::pair<long,std::string>>>& conditions,const std::list<std::pair<long,OrderBy>>& orderBy,Callback callback,Functor* functor) const;

        template<typename enumDef>
        bool Delete(const TableDefinition<enumDef>& table,const std::list<std::list<std::pair<long, std::string>>>& conditions);

        template<typename enumDef>
        bool ProcessList(std::string& querry,unsigned long& number,std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& attributes,const TableDefinition<enumDef>& table,const std::list<std::list<std::pair<long,std::string>>>& list,const std::string& negativeSeparator,const std::string& positiveSeparator) const;
        
        template<typename enumDef>
        bool ProcessList(std::string& querry,unsigned long& number,std::deque<std::pair<TableDefinitionBase::ValueDefinition,std::string>>& attributes,const TableDefinition<enumDef>& table,const std::list<std::pair<enumDef,std::string>>& list,const std::string& separator) const;
        
    protected:
        bool                            _DBIsCorrect;
        bool                            _DBIsCreated;   
        sqlite3*                        _SQLDB;
        bool                            _AccumulateQuerries;
        std::list<sqlite3_stmt*>        _Querries;   
};  

#include "Impl/SQLiteRegister.hxx"

#endif /* SQLITEREGISTER_HPP */

