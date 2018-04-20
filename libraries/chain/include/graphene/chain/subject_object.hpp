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
#include <graphene/chain/protocol/subject.hpp>
#include <graphene/chain/protocol/coin_ops.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <graphene/db/generic_index.hpp>

namespace graphene { namespace chain {
   class account_object;
   class database;
   using namespace graphene::db;

   /**
    * @class subject_statistics_object
    * @ingroup object
    * @ingroup implementation
    *
    * subject_object about statistics info
    */
   class subject_statistics_object : public graphene::db::abstract_object<subject_statistics_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_subject_statistics_object_type;

         subject_id_type  owner;

         std::map<string, uint64_t>   total; // <option, 人数>
         std::map<string, share_type> funds; // <option, 该option对应的总押注额>

         // create subject fee and vote subject fee
         share_type     fund_pool       =0;
         share_type     fee_pool        =0;

         // return subject creator income to encourage 
         share_type     subject_income  =0;
   };

   /*
    * subject_object kinds of time, eg create_time|vote_begin|vote_end|prediction_end|settle_time, 
    * for fsm status transition or judge vote result or settle funds etc
    */
   struct subject_status_expires
   {
        time_point_sec create_time;
        time_point_sec vote_begin;
        time_point_sec vote_end;
        time_point_sec prediction_end;
        time_point_sec settle_time;
   };

   /*
    * subject_object judge vote result, for settle funds etc
    * @param creator_is_win - subject creator is win
    * @param creator_win    - subject creator win funds
    * @param account_win    - the number of accounts is win
    * @param account_total  - the number of accounts is take part in subject vote total
    * @param funds_win      - the total of capital include win accounts
    * @param fund_pool      - the total of win funds, divided by win accounts 
    * 
    */
   struct subject_result
   {
        bool        creator_is_win;  //
        share_type  creator_win;      // 
        uint64_t    account_win;
        uint64_t    account_total;
        share_type  funds_win;
        share_type  fund_pool;
        share_type  fund_for_burn;  // 如果创建者输了，那么奖励资金池的5%，销毁处理 fund_for_burn为整数，包含小数部分，精度为8位小数.
   };

   /*
    *  @brief tracks subject information for vote
    *  @ingroup object
    * 
    *  All subject have a globally unique symbol id that controls how they are voted
    *
    */
   class subject_object: public graphene::db::abstract_object<subject_object>
   {
      public:
        static const uint8_t space_id = protocol_ids;
        static const uint8_t type_id  = subject_object_type;

        enum subject_status {
          none_status         = 0x0,
          create_status       = 0x1,
          vote_begin_status   = 0x2,
          vote_end_status     = 0x3,
          judge_status        = 0x4,
          settle_status       = 0x5,
          close_status        = 0x6,
          restore_status      = 0x7
        };


        string subject_name;  // subject random short name
        string article_url; //分析文章来源链接
        account_id_type   creator;
        share_type        deferred_fee; // deferred charges

        
        // subject time or subject status for fsm
        subject_status_expires status_expires;
        enum subject_status status   = none_status;

        string description;   // subject content description
        subject_template template_subject; // define subject template
        coin_price feed_price_result;
        optional<subject_result> result_subject;  // vote result for judge
        optional<map<string, string>>  exts; // extend_options


        /// The reference implementation records the subject_object's statistics in a separate object. This field contains the
        /// ID of that object.
        subject_statistics_id_type statistics;

        time_point_sec subject_creator_time()const
        { return status_expires.create_time; }

        time_point_sec vote_end_time()const
        { return status_expires.vote_end; }

        time_point_sec vote_begin_time()const
        { return status_expires.vote_begin; }

        time_point_sec vote_judge_time()const
        { return status_expires.prediction_end; }

        time_point_sec vote_settle_time()const
        { return status_expires.settle_time; }

        string creator_vote()const {
          return template_subject.vote;
        }

        bool enable_vote()const 
        {
          return (status == create_status || status == vote_begin_status);
        }
        bool enable_restore()const
        {
          return !(status == settle_status || status == close_status || status == restore_status);
        }

        static bool check_vote_options(const subject_template& template_subject);
        static subject_result judge_statistics(const subject_object& subject, const subject_statistics_object &subject_statistics, const string& select, uint64_t creator_trade_fee_percent, uint64_t creator_win_fund_percent);
        static std::map<string, share_type> get_funds_options(subject_template template_subject);
        static std::map<string, uint64_t> get_total_options(subject_template template_subject);
        static std::map<string, std::pair<string, string>> get_vote_options(subject_template template_subject);
        static std::map<string, share_type> increment_funds(std::map<string, share_type>& funds, const string& vote, share_type amount);
        static std::map<string, uint64_t> increment_total(std::map<string, uint64_t>& total, const string& vote, uint64_t incr);
        static std::vector<string> judge_result(const subject_template& template_subject, const string& judge);
        static share_type cut_percent_amount(share_type a, uint16_t p);
        static subject_rule::price_unit price_unit(const subject_template& template_subject);

   };


   struct subject_vote_result
   {
        asset             capital;  // the amount of vote
        asset             reward;   // the reward of total win
        int               judge;    // the result of vote 0,1
   };


   /*
    *  @brief tracks subject vote information
    *  @ingroup object
    * 
    *  All subject have a globally unique symbol id that controls how they are voted
    *
    */
   class subject_vote_object: public graphene::db::abstract_object<subject_vote_object>
   {
      public:
        static const uint8_t space_id = protocol_ids;
        static const uint8_t type_id  = subject_vote_object_type;

        account_id_type   voter;
        subject_id_type   subject_id;
        string            subject_name; // add name for hardy query vote history
        share_type        deferred_fee;
        time_point_sec    vote_time;  // current vote block time
        optional<map<string, string>>  exts; // extend_options

        optional<subject_vote_template> template_vote;
        optional<subject_vote_result>   vote_result;

        asset get_vote_amount()const
        { return template_vote->quantity; }
        string get_vote_option()const
        { return template_vote->my_vote; }

        time_point_sec subject_vote_time()const
        { return vote_time; }

   };

   class subject_event_object: public graphene::db::abstract_object<subject_event_object>
   {
      public:
        static const uint8_t space_id = protocol_ids;
        static const uint8_t type_id  = subject_event_object_type;

        account_id_type   oper;
        subject_id_type   subject_id;

        string            event; // update subject status event, create|vote_begin|vote_end|judge|settle|close etc
        variant           options;

        uint32_t          head_block_number = 0;
        time_point_sec    time;

        // record event operation status
        int             status;
        string          message;
        optional<map<string, string>>  exts; // extend_options

        string event_test(const string& key);

   };


   typedef enum subject_object::subject_status object_status_type;
   struct by_subject_name;
   struct by_subject_status;
   struct by_creator;
   struct by_creator_time;
   struct by_vote_end_time;
   //[lilianwen add 2017-10-31]
   struct by_prediction_end_time;
   //[end]

   typedef multi_index_container<
      subject_object,
      indexed_by<
          ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
          ordered_non_unique< tag<by_subject_name>, member<subject_object, string, &subject_object::subject_name> >,
          ordered_non_unique< tag<by_subject_status>, member<subject_object, object_status_type, &subject_object::status> >,
          ordered_non_unique< tag<by_creator>, member<subject_object, account_id_type, &subject_object::creator> >,
          ordered_non_unique< tag<by_creator_time>,
              const_mem_fun<subject_object, time_point_sec, &subject_object::subject_creator_time>
          >,
          ordered_non_unique< tag<by_vote_end_time>,
              const_mem_fun<subject_object, time_point_sec, &subject_object::vote_end_time>
          >,
		  //[lilianwen add 2017-10-31]
		  ordered_non_unique< tag<by_prediction_end_time>,
		  const_mem_fun<subject_object, time_point_sec, &subject_object::vote_judge_time>
		  >
		  //[end]
      >
   > subject_object_multi_index_type;
   typedef generic_index<subject_object, subject_object_multi_index_type> subject_index;


   struct by_subject_id;
   struct by_voter;
   struct by_vote_time;
   typedef multi_index_container<
      subject_vote_object,
      indexed_by<
          ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
          ordered_non_unique< tag<by_subject_id>, member<subject_vote_object, subject_id_type, &subject_vote_object::subject_id> >,
          ordered_non_unique< tag<by_voter>, member<subject_vote_object, account_id_type, &subject_vote_object::voter> >,
          ordered_non_unique< tag<by_vote_time>,
              const_mem_fun<subject_vote_object, time_point_sec, &subject_vote_object::subject_vote_time>
          >
      >
   > subject_vote_index_multi_index_type;
   typedef generic_index<subject_vote_object, subject_vote_index_multi_index_type> subject_vote_index;

   struct by_event_operator;
   typedef multi_index_container<
      subject_event_object,
      indexed_by<
          ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
          ordered_non_unique< tag<by_subject_id>, member<subject_event_object, subject_id_type, &subject_event_object::subject_id> >,
          ordered_non_unique< tag<by_event_operator>, member<subject_event_object, account_id_type, &subject_event_object::oper> >
      >
   > subject_event_index_multi_index_type;
   typedef generic_index<subject_event_object, subject_event_index_multi_index_type> subject_event_index;


   //[lilianwen add 2017-10-25]
   struct full_subject_vote_object
   {
	   //投票的主题描述
	   object_id_type         subject_id;
	   string                 subject_creator;
	   subject_status_expires status_expires;//描述投票截止时间等
	   object_status_type status;//投票状态（进行中，已截止）
	   string description;   // subject content description
	   subject_template template_subject;
	   coin_price feed_price_result;
	   optional<subject_result> result;//如果裁决结束，则需要返回裁决结果
	   string article_url;//[lilianwen add 2017-12-7]
	   share_type deferred_fee; //[lilianwen add 2017-12-7] deferred charges


	   //投票的动态统计信息，投票各方人数和资金数
	   std::map<string, uint64_t>   total;//投票各方的人数
	   std::map<string, share_type> funds;//投票各方的资金数

	   //我的账号是否投过这个票
	   vector<object_id_type> subject_vote_id;
	   subject_statistics_id_type statistics;
   };

   struct query_condition
   {
	   uint32_t                       start;    //返回的起始序号，从1开始
	   uint32_t                       limit;    //本次查询最大数量
	   time_point_sec                 start_time;
	   time_point_sec                 end_time;
	   uint32_t                       direction;//1从小到大，其他值从大到小
	   string                         order_by; //"vote_end_time" | "create_time" | "prediction_end_time"
	   subject_rule::price_unit       platform_quote_base;
	   string                         status;   //"all" | "create_status" | "vote_end_status" | "prediction_status"
	   string                         account_name_or_id;//网页显示的当前账户名或ID
   };

   //[end]

   

} } // graphene::chain

FC_REFLECT(graphene::chain::subject_status_expires, (create_time)(vote_begin)(vote_end)(prediction_end)(settle_time))
FC_REFLECT(graphene::chain::subject_vote_result, (capital)(reward)(judge))
FC_REFLECT(graphene::chain::subject_result, (creator_is_win)(creator_win)(account_win)(account_total)(funds_win)(fund_pool)(fund_for_burn))
FC_REFLECT_ENUM(graphene::chain::subject_object::subject_status, 
  (none_status)(create_status)(vote_begin_status)(vote_end_status)(judge_status)(settle_status)(close_status)(restore_status))


FC_REFLECT_DERIVED( graphene::chain::subject_statistics_object, (graphene::db::object),
                    (owner)(total)(funds)(fund_pool)(fee_pool)(subject_income)
                  )

FC_REFLECT_DERIVED( graphene::chain::subject_object, (graphene::db::object),
                    (subject_name)(article_url)(creator)(deferred_fee)(status_expires)
                    (status)(description)(template_subject)(feed_price_result)(result_subject)(statistics)(exts)
                  )

FC_REFLECT_DERIVED( graphene::chain::subject_vote_object, (graphene::db::object),
                    (voter)(subject_id)(subject_name)(deferred_fee)(vote_time)(template_vote)(vote_result)(exts)
                  )

FC_REFLECT_DERIVED( graphene::chain::subject_event_object, (graphene::db::object),
                    (oper)(subject_id)(event)(options)(head_block_number)(time)(status)(message)(exts)
                  )

//[lilianwen add 2017-10-25]
FC_REFLECT( graphene::chain::full_subject_vote_object, (subject_id)(subject_creator)(status_expires)(status)(description)(template_subject)(feed_price_result)(result)(article_url)(deferred_fee)(total)(funds)(subject_vote_id)(statistics))
FC_REFLECT( graphene::chain::query_condition, (start)(limit)(start_time)(end_time)(direction)(order_by)(platform_quote_base)(status)(account_name_or_id) )
//[end]
