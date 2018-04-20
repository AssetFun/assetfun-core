/*
 * Copyright (c) 2015 AssetFun, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <graphene/chain/protocol/operations.hpp>
#include <graphene/chain/evaluator.hpp>

namespace graphene { namespace chain {

   class database;
   class configurator
   {
   public:
	   virtual ~configurator(){}

	   virtual void_result evaluate(const module_cfg_operation& o) = 0;

	   virtual void_result apply(const module_cfg_operation& o) = 0;

	   virtual void set_database(database* db){_db = db;}
	protected:
		database* _db;
   };

   template<typename ConfiguratorType>
   class configurator_impl : public configurator
   {
   public:
	   void_result evaluate(const module_cfg_operation& o) override
	   {
			//module specific evaluation
		    ConfiguratorType configurator(*_db);
		    return configurator.evaluate(o);
	   }

	   void_result apply(const module_cfg_operation& o) override
	   {
			//module specific apply
		    ConfiguratorType configurator(*_db);
		    return configurator.apply(o);
	   }
   };

   template<typename DerivedConfiguratorType>
   class module_configurator
   {
   public:
	   module_configurator(database& db): _db(db) {};
	   virtual ~module_configurator(){};

	   //module specific evaluation
	   virtual void_result evaluate(const module_cfg_operation& o) = 0;

	   //module specific apply
	   virtual void_result apply(const module_cfg_operation& o) = 0;

	protected:
		database& _db;
   };

   class coin_configurator : public module_configurator<coin_configurator>
   {
   public:
	   static const string name;

	   coin_configurator(database& db): module_configurator(db){};
	   virtual ~coin_configurator(){};

	   void_result evaluate(const module_cfg_operation& o) override;

	   void_result apply(const module_cfg_operation& o) override;
        struct one_platform
        {
           string platform_id;
           map<string, string> quote_bases; //e.g. {"BTC/CNY":"0","BTC/USD":"1","BTC/USD":"2"}
           string status; //交易对状态 "1":可见可用, "2":可见不可用, "3":不可见不可用(即下架)
           string platform_en;
           string platform_cn;
           string time_zone;
        };

	   struct platforms_config
	   {
	   	   vector<one_platform> platform_info;
	   };
	   platforms_config cfg;
   };

   // TOKEN参数配置模块
   class token_configurator : public module_configurator<token_configurator>
   {
   public:
        static const string name;

        token_configurator(database& db): module_configurator(db){};
        virtual ~token_configurator(){};

        void_result evaluate(const module_cfg_operation& o) override;

        void_result apply(const module_cfg_operation& o) override;
   };

   // 敏感词参数配置模块
   class word_configurator : public module_configurator<word_configurator>
   {
   public:
        static const string name;

        word_configurator(database& db): module_configurator(db){};
        virtual ~word_configurator(){};

        void_result evaluate(const module_cfg_operation& o) override;

        void_result apply(const module_cfg_operation& o) override;
   };
} } // graphene::chain
