/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, token to the following conditions:
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
#include <graphene/chain/protocol/token.hpp>
#include <graphene/chain/protocol/coin_ops.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <graphene/db/generic_index.hpp>

namespace graphene { namespace chain {
   class account_object;
   class database;
   using namespace graphene::db;

   struct return_asset_record
   {
      return_asset_record(){}

      return_asset_record( time_point_sec t, asset a )
      :time(t),return_asset(a){}

      time_point_sec time;  // 返还时间
      asset return_asset; // 返还资产
   };
   
   /**
    * 项目募集明细统计
    */
   class token_statistics_object : public graphene::db::abstract_object<token_statistics_object>
   {
      public:
        static const uint8_t space_id = implementation_ids;
        static const uint8_t type_id  = impl_token_statistics_object_type;

        token_id_type                    token_id; // 通证(众筹项目)id
        uint64_t                         buyer_number = 0;    //认购总人数
        std::set<account_id_type>        buyer_ids;       //认购人id集合, 认购总人数=buyer_ids.size()
        share_type                       actual_core_asset_total                   = 0; //实际认购总资金(核心资产，单位AFT)，包含8位小数
        share_type                       actual_buy_total                          = 0; //实际认购的总用户资产，包含8位小数
        share_type                       actual_buy_percentage                     = 0; //众筹募集到的资金比例，保留2位小数。如值为1234，表示12.34%
        share_type                       actual_not_buy_total                      = 0; //实际没认购的总用户资产，actual_not_buy_amount = token_template::plan_collect_amount - actual_buy_amount
        asset                            has_returned_guaranty_core_asset; //已经返还的发行人抵押的核心资产数，包含8位小数
        asset                            has_returned_issuer_reserved_asset; //已经返还的发行人预留的用户资产数，包含8位小数
        std::vector<return_asset_record> return_guaranty_core_asset_detail; // 发行人抵押的核心资产(AFT)的解冻返还明细, <时间, 解冻返还的用户资产>，目前是一次性返还，所以只有一个记录
        std::vector<return_asset_record> return_issuer_reserved_asset_detail; // 发行人预留的用户资产的解冻返还明细, <时间, 解冻返还的用户资产>，每一次返还(每月返还一次)生成一个记录

         /*
         token_object::template_parameter::plan_buy_total = token_statistics_object::actual_buy_total + token_statistics_object::actual_not_buy_total
         token_object::max_supply = token_object::template_parameter::plan_buy_total + 发行人预留的用户资产
         实际没认购资金的处理：
         对于没有募集成功的那部分用户资产，如果是按比例分发给已经参与的人，则更新相应的token_buy_object
         */
   };

   /*
    * 项目状态
    */
   struct token_status_expires
   {
        time_point_sec create_time; // 用户资产发行时间(众筹创建时间)
        time_point_sec phase1_begin; // 认购第1阶段开始时间
        time_point_sec phase1_end; // 认购第1阶段结束时间
        time_point_sec phase2_begin; // 认购第2阶段开始时间
        time_point_sec phase2_end; // 认购第2阶段结束时间，如果没有提前结束众筹，phase2_end也是众筹结算时间
        time_point_sec settle_time; // 实际结算时间(实际认购结束时间)
        time_point_sec next_return_guaranty_core_asset_time   = time_point_sec(0); // 返还发行人抵押的核心资产(AFT)下一个返还时间  
        time_point_sec next_return_issuer_reserved_asset_time = time_point_sec(0); // 返还发行人预留的用户资产下一个返还时间
        time_point_sec return_guaranty_core_asset_end         = time_point_sec(0); // 返还发行人抵押的核心资产(AFT)结束时间
        time_point_sec return_issuer_reserved_asset_end       = time_point_sec(0); // 返还发行人预留的用户资产结束时间
        time_point_sec return_asset_end; // 返还众筹抵押的核心资产(AFT)和发行人预留的用户资产结束时间, return_asset_end = max(return_guaranty_core_asset_end, return_issuer_reserved_asset_end)
   };

   /*
    * 项目募集结果
    */
   struct token_result
   {
        bool        is_succeed = false;  //募集是否成功
        //share_type  actual_buy_amount; //实际认购总资金，单位AFT, 为整数，包含小数部分，精度为8位小数
        //share_type  actual_not_buy_amount; //实际没认购总资金，单位AFT， actual_not_buy_amount = token_template::plan_collect_amount - actual_buy_amount
        //uint32_t    actual_buy_percentage; //募集到的资金比例，保留2位小数。如值为1234，表示12.34%
   };

   /*
    *  @brief tracks token information
    *  @ingroup object
    * 
    *  All token have a globally unique symbol id that controls how they are voted
    *
    */
   class token_object: public graphene::db::abstract_object<token_object>
   {
      public:
        static const uint8_t space_id = protocol_ids;
        static const uint8_t type_id  = token_object_type;

        enum token_status {
          none_status             = 0x0, // 初始状态
          create_status           = 0x1, // 通证已创建
          phase1_begin_status     = 0x2, // 认购第1阶段开始
          phase1_end_status       = 0x3, // 认购第1阶段结束
          phase2_begin_status     = 0x4, // 认购第2阶段开始
          phase2_end_status       = 0x5, // 认购第2阶段结束
          settle_status           = 0x6, // 众筹已结算
          return_asset_end_status = 0x7, // 返还众筹抵押的核心资产(AFT)和发行人预留的用户资产，对应token_status_expires::return_asset_end
          close_status            = 0x8, // 众筹结束
          restore_status          = 0x9  // 众筹失败，资金已回退
        };

        enum token_control {// 通证查询时的显示控制
          available               = 0x0, // 初始状态，完全允许显示
          description_forbidden   = 0x1, // 通证(众筹项目)描述被禁止显示
          unavailable             = 0x2, // 完全不允许显示
          end_of_token_control    = unavailable, // 完全不允许显示
        };

        account_id_type           issuer; //用户资产发行人/众筹项目发起人
        token_template            template_parameter; // define token template

        string                    upper_case_asset_name; //大写通证名称
        asset_id_type             user_issued_asset_id; //用户资产id
        share_type                buy_succeed_min_amount; //用户资产募集成功的最少数量，包含8位小数，用户资产募集成功的最少数量= token_template::collect_amout * token_template::collect_succeed_min_percent
        share_type                issuer_reserved_asset_total = 0; //发行人预留的用户资产，包含8位小数。issuer_reserved_asset_total = template_parameter::max_supply - template_parameter::plan_buy_total
        share_type                each_period_return_guaranty_core_asset    = 0; //每一期应返还的发行人抵押的核心资产数，包含8位小数
        share_type                each_period_return_issuer_reserved_asset  = 0; //每一期应返还的发行人预留的用户资产数，包含8位小数
        share_type                guaranty_credit = 0; //抵押信用，抵押信用 = 抵押的核心资产数(为避免溢出，不包含8位小数) * 抵押月数

        share_type                deferred_fee; // deferred charges
        uint8_t                   permission = 0; //权重，标识是否可以开始认购
        token_control             control = available;

        
        // token time or token status for fsm
        token_status_expires    status_expires;
        enum token_status       status   = none_status;

        token_result            result;  // 众筹结果
        
        /// The reference implementation records the token_object's statistics in a separate object. This field contains the
        /// ID of that object.
        token_statistics_id_type statistics;

        optional<map<string, string>>  exts; // extend_options

        time_point_sec create_time()const
        { return status_expires.create_time; }

        time_point_sec phase1_begin_time()const
        { return status_expires.phase1_begin; }

        time_point_sec phase1_end_time()const
        { return status_expires.phase1_end; }

        time_point_sec phase2_begin_time()const
        { return status_expires.phase2_begin; }

        time_point_sec phase2_end_time()const
        { return status_expires.phase2_end; }

        time_point_sec settle_time()const
        { return status_expires.settle_time; }

        time_point_sec next_return_guaranty_core_asset_time()const
        { return status_expires.next_return_guaranty_core_asset_time; }

        time_point_sec next_return_issuer_reserved_asset_time()const
        { return status_expires.next_return_issuer_reserved_asset_time; }

        time_point_sec return_guaranty_core_asset_end_time()const
        { return status_expires.return_guaranty_core_asset_end; }

        time_point_sec return_issuer_reserved_asset_end_time()const
        { return status_expires.return_issuer_reserved_asset_end; }

        time_point_sec return_asset_end_time()const
        { return status_expires.return_asset_end; }

        bool enable_buy()const 
        {
          return (status == phase1_begin_status) || (status == phase2_begin_status);
        }
        bool enable_restore()const
        {
          return !(status == settle_status || status == return_asset_end_status || status == close_status || status == restore_status);
        }

        string get_upper_case_asset_name()const 
        {
          return upper_case_asset_name;
        }

        string get_asset_symbol()const 
        {
          return template_parameter.asset_symbol;
        }

        time_point_sec get_create_time()const 
        {
          return status_expires.create_time;
        }

        static share_type cut_percent_amount(share_type a, uint16_t p);
   };


   struct token_buy_result
   {  
        asset      pay_base_amount;  // 认购支付的核心资产，单位AFT，包含8位小数
        asset      buy_quote_amount; // 认购用户资产数量，包含8位小数
        asset      reward_quote_amount; // 对于没有募集成功的那部分用户资产，如果是按比例分发给已经参与的人，reward_quote_amount记录当前认购者可以分派的用户资产数量，包含8位小数
   };


   /*
    *  @brief tracks token buy information
    *  @ingroup object
    * 
    *  All token have a globally unique symbol id that controls how they are voted
    *
    */
   class token_buy_object: public graphene::db::abstract_object<token_buy_object>
   {
      public:
        static const uint8_t space_id = protocol_ids;
        static const uint8_t type_id  = token_buy_object_type;

        account_id_type                     buyer;
        token_id_type                       token_id;
        time_point_sec                      buy_time;  // current buy block time
        token_buy_template                  template_parameter;
        token_buy_result                    buy_result;
        share_type                          deferred_fee;

        optional<map<string, string>>       exts; // extend_options


        share_type get_buy_quantity()const
        { return template_parameter.buy_quantity; }

        time_point_sec get_buy_time()const
        { return buy_time; }
   };


   class token_event_object: public graphene::db::abstract_object<token_event_object>
   {
      public:
        static const uint8_t space_id = protocol_ids;
        static const uint8_t type_id  = token_event_object_type;

        account_id_type   oper;
        token_id_type   token_id;

        string            event; // update token status event, create|phase1_begin|phase1_end|phase2_begin|phase2_end|settle|return_asset_end|restore|set_token_control|close etc
        variant           options;

        uint32_t          head_block_number = 0;
        time_point_sec    time;
        string            content = "";

        // record event operation status
        int             status;
        string          message; //事件操作结果
        optional<map<string, string>>  exts; // extend_options

        string event_test(const string& key);

   };


   typedef enum token_object::token_status token_object_status_type;
   struct by_token_status;
   struct by_issuer;
   struct by_upper_case_asset_name;
   struct by_asset_symbol;
   struct by_create_time;
   struct by_actual_core_asset_total;
   struct by_end_time;
   struct by_guaranty_credit;

   typedef multi_index_container<
      token_object,
      indexed_by<
          ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
          ordered_non_unique< tag<by_token_status>, member<token_object, token_object_status_type, &token_object::status> >,
          ordered_non_unique< tag<by_issuer>, member<token_object, account_id_type, &token_object::issuer> >,
          ordered_non_unique< tag<by_upper_case_asset_name>, const_mem_fun<token_object, string, &token_object::get_upper_case_asset_name> >,
          ordered_non_unique< tag<by_asset_symbol>, const_mem_fun<token_object, string, &token_object::get_asset_symbol> >,
          ordered_non_unique< tag<by_create_time>, const_mem_fun<token_object, time_point_sec, &token_object::get_create_time> >,
		  ordered_non_unique< tag<by_end_time>, const_mem_fun<token_object, time_point_sec, &token_object::phase2_end_time> >,
		  ordered_non_unique< tag<by_guaranty_credit>, member<token_object, share_type, &token_object::guaranty_credit> >
          //ordered_non_unique< tag<by_collected_core_asset>, const_mem_fun<token_object, share_type, &token_object::get_actual_core_asset_total> >,
        //>,
		  >
   > token_object_multi_index_type;
   typedef generic_index<token_object, token_object_multi_index_type> token_index;

/*
      typedef multi_index_container<
      token_object,
      indexed_by<
          ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >
      >
   > token_object_multi_index_type;
   typedef generic_index<token_object, token_object_multi_index_type> token_index;
*/

   struct by_token_id;
   struct by_buyer;
   struct by_buy_time;
   typedef multi_index_container<
      token_buy_object,
      indexed_by<
          ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
          ordered_non_unique< tag<by_token_id>, member<token_buy_object, token_id_type, &token_buy_object::token_id> >,
          ordered_non_unique< tag<by_buyer>, member<token_buy_object, account_id_type, &token_buy_object::buyer> >,
          ordered_non_unique< tag<by_buy_time>,
              member<token_buy_object, time_point_sec, &token_buy_object::buy_time>
          >
      >
   > token_buy_index_multi_index_type;
   typedef generic_index<token_buy_object, token_buy_index_multi_index_type> token_buy_index;

   struct by_event_operator;
   typedef multi_index_container<
      token_event_object,
      indexed_by<
          ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
          ordered_non_unique< tag<by_token_id>, member<token_event_object, token_id_type, &token_event_object::token_id> >,
          ordered_non_unique< tag<by_event_operator>, member<token_event_object, account_id_type, &token_event_object::oper> >
      >
   > token_event_index_multi_index_type;
   typedef generic_index<token_event_object, token_event_index_multi_index_type> token_event_index;

   //[lilianwen add 2018-1-30]
   struct by_token_id;
   struct by_buyer_number;
   struct by_actual_core_asset_total;
   typedef multi_index_container<
      token_statistics_object,
      indexed_by<
          ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
          ordered_non_unique< tag<by_token_id>, member< token_statistics_object, token_id_type, &token_statistics_object::token_id > >,
          ordered_non_unique< tag<by_buyer_number>, member<token_statistics_object, uint64_t, &token_statistics_object::buyer_number> >,
          ordered_non_unique< tag<by_actual_core_asset_total>, member<token_statistics_object, share_type, &token_statistics_object::actual_core_asset_total> >
      >
   > token_statistics_index_multi_index_type;
   typedef generic_index<token_statistics_object, token_statistics_index_multi_index_type> token_statistics_index;

   struct token_query_condition
   {
	   uint32_t                       start;    //返回的起始序号，从1开始
	   uint32_t                       limit;    //本次查询最大数量
	   time_point_sec                 start_time;
	   time_point_sec                 end_time;
	   string                         order_by; //"create_time" | "end_time" |"buy_amount" | "buyer_number" | "guaranty_amount"
	   string                         status;   //"all" | "create_status" | "end_status"
	   string                         my_account;//网页显示的当前账户名或ID
	   set<string>                    extra_query_fields;//额外的查询内容
   };

   struct token_brief
   {
	   object_id_type token_id;
	   string         issuer;
	   uint32_t       status;//token的状态,第几阶段，认购结束等
	   string         logo_url;
	   string         asset_name;
	   string         asset_symbol;
	   string         max_supply;//token的最大供应量
	   string         plan_buy_total;
	   time_point_sec create_time;
	   time_point_sec phase1_begin; // 认购第1阶段开始时间
	   time_point_sec phase1_end; // 认购第1阶段结束时间
	   time_point_sec phase2_begin; // 认购第2阶段开始时间
	   time_point_sec phase2_end;
	   time_point_sec settle_time;//token的结算时间
	   share_type     actual_buy_amount;//所有参与的用户已经认购的用户资产数量，这个考虑一下怎么算
	   share_type     actual_core_asset_total;//所有参与的用户已经募集的AFT数量
	   uint32_t       buyer_number;//所有参与的用户人数/yer_ids.size()
	   asset          guaranty_core_asset_amount;//抵押AFT数量
	   share_type     guaranty_credit;//抵押信用
     uint32_t       control;
	   string         brief;//token的摘要简介
	   uint32_t       buy_count;  //为0表示当前账号没有认购，否则就是认购次数
	   vector<token_buy_template> my_participate;//我参与的情况
     bool           need_raising;//是否在Assetfun平台上募集
	   map<string,string>    extend_field;//扩展字段
   };

   struct token_detail
   {
	   object_id_type  token_id;
	   string          type;
	   string          subtype;
	   string     issuer;//token发行人
	   uint32_t   control;
	   asset_id_type   user_issued_asset_id; //用户资产id
	   uint32_t   status;
	   share_type guaranty_credit;
	   string     logo_url;
	   string     asset_name;
	   string     asset_symbol;
	   string     max_supply;
	   string     plan_buy_total;//要改成plan_buy_total
	   uint64_t   buy_succeed_min_percent; //后面要改成buy_succeed_min_percent
	   string     brief;   

	   uint32_t   buy_count;//为0表示当前账号没有认购，否则就是认购次数
	   share_type actual_core_asset_total;
	   share_type actual_buy_total;
	   share_type actual_buy_percentage;
	   share_type actual_not_buy_total; 
	   std::vector<return_asset_record> return_guaranty_core_asset_detail;
	   std::vector<return_asset_record> return_issuer_reserved_asset_detail;
	   uint32_t   buyer_number;
	   std::map<string, token_rule::each_buy_phase_setting>  buy_phases;//每个阶段的起始结束时间存放在这里
	   asset           guaranty_core_asset_amount; 
	   uint32_t        guaranty_core_asset_months; 
	   uint8_t         issuer_reserved_asset_frozen_months;
	   uint8_t         not_buy_asset_handle; 
	   vector<string>  whitelist;
	   string          description; //通证(众筹项目)描述
	   time_point_sec create_time; // 用户资产发行时间(众筹创建时间)
	   time_point_sec phase1_end; // 认购第1阶段结束时间
	   time_point_sec settle_time; // 实际结算时间(实际认购结束时间)

	   time_point_sec next_return_guaranty_core_asset_time; // 返还发行人抵押的核心资产(AFT)下一个返还时间  
	   time_point_sec next_return_issuer_reserved_asset_time; // 返还发行人预留的用户资产下一个返还时间
	   time_point_sec return_guaranty_core_asset_end; // 返还发行人抵押的核心资产(AFT)结束时间
	   time_point_sec return_issuer_reserved_asset_end; // 返还发行人预留的用户资产结束时间
	   time_point_sec return_asset_end; 

	   vector<token_buy_template> my_participate;//我参与的情况，购买了多少次，每次多少个等等信息
	   optional<token_result>     result;// 众筹结果
	   map<string, string>        customized_attributes;
     bool           need_raising;//是否在Assetfun平台上募集
	   map<string,string>         extend_field;//扩展字段
   };
   //[end]

} } // graphene::chain

FC_REFLECT(graphene::chain::token_status_expires, (create_time)(phase1_begin)(phase1_end)(phase2_begin)(phase2_end)(settle_time)(next_return_guaranty_core_asset_time)(next_return_issuer_reserved_asset_time)(return_asset_end))
FC_REFLECT(graphene::chain::token_buy_result, (pay_base_amount)(buy_quote_amount)(reward_quote_amount))
FC_REFLECT(graphene::chain::token_result, (is_succeed))
FC_REFLECT_ENUM(graphene::chain::token_object::token_status, 
  (none_status)(create_status)(phase1_begin_status)(phase1_end_status)(phase2_begin_status)(phase2_end_status)(settle_status)(return_asset_end_status)(close_status)(restore_status))
FC_REFLECT_ENUM(graphene::chain::token_object::token_control, 
  (available)(description_forbidden)(unavailable))
FC_REFLECT(graphene::chain::return_asset_record, (time)(return_asset))

FC_REFLECT_DERIVED( graphene::chain::token_statistics_object, (graphene::db::object),
                    (token_id)(buyer_number)(buyer_ids)(actual_core_asset_total)(actual_buy_total)(actual_buy_percentage)(actual_not_buy_total)(has_returned_guaranty_core_asset)(has_returned_issuer_reserved_asset)
                    (return_guaranty_core_asset_detail)(return_issuer_reserved_asset_detail)
                  )

FC_REFLECT_DERIVED( graphene::chain::token_object, (graphene::db::object),
                    (issuer)(template_parameter)(user_issued_asset_id)(buy_succeed_min_amount)(issuer_reserved_asset_total)(each_period_return_guaranty_core_asset)
                    (each_period_return_issuer_reserved_asset)(guaranty_credit)(deferred_fee)(permission)(control)(status_expires)(status)(result)(statistics)(exts)
                  )

FC_REFLECT_DERIVED( graphene::chain::token_buy_object, (graphene::db::object),
                    (buyer)(token_id)(deferred_fee)(buy_time)(template_parameter)(buy_result)(exts)
                  )

FC_REFLECT_DERIVED( graphene::chain::token_event_object, (graphene::db::object),
                    (oper)(token_id)(event)(options)(head_block_number)(time)(status)(message)(exts)
                  )


FC_REFLECT( graphene::chain::token_query_condition, 
			(start)(limit)(start_time)(end_time)(order_by)(status)(my_account)(extra_query_fields)
		  )

FC_REFLECT( graphene::chain::token_brief, 
		  (token_id)(issuer)(status)(logo_url)(asset_name)(asset_symbol)(max_supply)(plan_buy_total)(create_time)(phase1_begin)(phase1_end)(phase2_begin)(phase2_end)(settle_time)(actual_buy_amount)
		  (actual_core_asset_total)(buyer_number)(guaranty_core_asset_amount)(guaranty_credit)(control)(brief)(buy_count)(my_participate)(need_raising)(extend_field)
		  )

FC_REFLECT( graphene::chain::token_detail, 
		  (token_id)(type)(subtype)(issuer)(control)(user_issued_asset_id)(guaranty_credit)(status)(logo_url)(asset_name)(asset_symbol)(max_supply)(plan_buy_total)(buy_succeed_min_percent)(brief)(buy_count)
		  (actual_core_asset_total)(actual_buy_total)(actual_buy_percentage)(actual_not_buy_total)(return_guaranty_core_asset_detail)(return_issuer_reserved_asset_detail)(buyer_number)
		  (buy_phases)(guaranty_core_asset_amount)(guaranty_core_asset_months)(issuer_reserved_asset_frozen_months)(not_buy_asset_handle)(whitelist)(description)(create_time)
		  (phase1_end)(settle_time)(next_return_guaranty_core_asset_time)(my_participate)(result)(next_return_issuer_reserved_asset_time)(return_guaranty_core_asset_end)(return_issuer_reserved_asset_end)
		  (return_asset_end)(customized_attributes)(need_raising)(extend_field)
		  )