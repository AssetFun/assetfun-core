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
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>


namespace graphene { namespace chain { 


   struct subject_rule
   {
      struct price_unit
      {
         string platform_id = ""; // e.g. "1000001"，第7位数以上表示交易类型，1 数字货币，2 黄金，3 股票。右边6位数表示具体的平台id。首位和末位数字都不能为0。"0000000"为保留id
         string quote_base  = "";
      };
      struct option_value {
         string alpha;
         string beta;
      };
      enum rule_type {
         rule_none      = 0x0,
         dual_radio     = 0x1,
         mutil_radio    = 0x2,
         mutil_check    = 0x3,
         continuous_interval = 0x4,
         rule_all = dual_radio|mutil_radio|mutil_check|continuous_interval
      };
   };

   /*
    * subject tempalte:
    * {
    *  "event": "price", 
    *  "unit": {"quote": "BTC", "base": "CNY"}, 
    *  "type": "dual_radio", 
    *  "vote": "0", 
    *  "after": "2018-10-10T12:00", 
    *  "options": {"0":{"0":"3000"}, "1":{"3000": "122222000"}
    *  }
    * 
    */
   struct subject_template {

      string   event;
      subject_rule::price_unit  unit;    //struct subject_rule::price_unit unit;
      time_point_sec after;
      string   vote;
      string   type;    // dual_radio|mutil_radio|mutil_check|continuous_interval
      std::map<string, subject_rule::option_value>  options; //区间值为浮点数字符串，包含小数部分，精度为8位小数，如"1.23456789"
      std::map<string, subject_rule::option_value>  title;

      /// Perform internal consistency checks.
      /// @throws fc::exception if any check fails
      void validate()const;
   };

   struct subject_content
   {
      string                  description;
      struct subject_template template_subject;
   };

   struct subject_options
   {
      time_point_sec create_time;
      time_point_sec prediction_interval;
      asset creator_vote;
      
      void validate()const;
   };

   struct extend_options
   {
      int opt;
      void validate()const;
   };

   struct query_for_subject_publish_paramter
   {
      time_point_sec create_time;
      time_point_sec prediction_interval; //prediction_end_time
      time_point_sec vote_begin_time;
      time_point_sec vote_end_time;
      time_point_sec settle_time;
      asset create_fee;
      
      void validate()const;
   };

   /*
    * vote template:
    * { "quantity": {"amount": 5, "asset_id": "1.3.0"}, 
    *   "vote": 1, 
    *   "unit": {"BTC/CNY"}，
    * }
    */
   struct subject_vote_template
   {
      // vote template
      asset                      quantity;   // vote amount
      string                     creator_vote;  // vote option, relate to subject_template.options
	  string                     my_vote;       // vote option, relate to subject_template.options
      //subject_rule::price_unit	 unit;       // vote coin unit BTC/CNY

   };

   /**
    * @class  subject_publish_operation
    * @brief  the subject vote instrument define kinds of event for prediction
    * @ingroup operations
    * 
    * ...fee : base fee & variable fee per hour 
    */
   struct subject_publish_operation : public base_operation
   {
   	struct fee_parameters_type
   	{
         uint64_t basic_fee       = 5 * GRAPHENE_BLOCKCHAIN_PRECISION;       //基础费用5个代币
   	  	 uint64_t price_per_hour  = 0.05 * GRAPHENE_BLOCKCHAIN_PRECISION;    // 0.05AFT/hour
   	};

   	asset           		fee;

   	// This account must sign and pay the fee for this operation. Later, this account may update the asset
      account_id_type       creator;

      // subject random short name
      string subject_name;
      string article_url = ""; //分析文章来源链接

      optional<subject_content> content;

      // subject profile setting
      optional<subject_options>      opts;
      optional<extend_options>       exts; // hold use

      extensions_type      extensions;

      asset creator_vote()const
      {
         return opts->creator_vote;
      }

      uint64_t get_pred_interval()const
      {
         FC_ASSERT( opts->prediction_interval >= opts->create_time );
         return (opts->prediction_interval - opts->create_time).to_seconds(); // second
      }

      account_id_type fee_payer()const { return creator; }
      void            validate()const;
      share_type      calculate_fee( const fee_parameters_type& k )const;
   };

   /**
    * @class  subject_vote_operation
    * @brief  the subject vote instrument record vote operation for prediction
    * @ingroup operations
    * 
    * ...
    */
   struct subject_vote_operation : public base_operation
   {
      struct fee_parameters_type
      {
         uint64_t fee_percent_of_vote_amount   = 80; //0.8%;
      };

      asset             fee;
      account_id_type   voter;
      subject_id_type   subject_id;

      // vote template
      subject_vote_template   template_vote;

      asset get_vote_amount()const 
      {
         return template_vote.quantity;
      }

      string get_vote_option()const { return template_vote.my_vote; }

      account_id_type fee_payer()const { return voter; }
      void            validate()const;
      share_type      calculate_fee( const fee_parameters_type& k )const;
   };

   /**
    * @class  subject_event_operation
    * @brief  the subject vote instrument update subject_object status for prediction, 
    *         and handle event for subject judge\settle\close etc
    * @ingroup operations
    * 
    * ...
    */
   struct subject_event_operation : public base_operation
   {
      struct fee_parameters_type
      {
         uint64_t fee       = 0;
      };

      asset             fee;
      account_id_type   oper; // witness or committee or system ?
      subject_id_type   subject_id;
      string            event; //
      //asset             real; // predition end feed price 
      variant           options;

      account_id_type fee_payer()const { return oper; }
      void            validate()const;
      share_type      calculate_fee( const fee_parameters_type& k )const;
   };

} } // graphene::chain

FC_REFLECT(graphene::chain::subject_rule::price_unit, (platform_id)(quote_base))
FC_REFLECT(graphene::chain::subject_rule::option_value, (alpha)(beta))

FC_REFLECT_ENUM(graphene::chain::subject_rule::rule_type, (rule_none)(dual_radio)(mutil_radio)(mutil_check)(continuous_interval)(rule_all))
FC_REFLECT(graphene::chain::subject_rule, )


FC_REFLECT(graphene::chain::subject_template, (event)(unit)(after)(vote)(type)(options))
FC_REFLECT(graphene::chain::subject_content, (description)(template_subject))
FC_REFLECT(graphene::chain::subject_options, (create_time)(prediction_interval)(creator_vote))
FC_REFLECT(graphene::chain::extend_options, (opt))
FC_REFLECT(graphene::chain::query_for_subject_publish_paramter, (create_time)(prediction_interval)(vote_begin_time)(vote_end_time)(settle_time)(create_fee) )
FC_REFLECT(graphene::chain::subject_vote_template, (quantity)(creator_vote)(my_vote))

FC_REFLECT( graphene::chain::subject_publish_operation::fee_parameters_type, (basic_fee)(price_per_hour) )
FC_REFLECT( graphene::chain::subject_publish_operation, (fee)(creator)(subject_name)(article_url)(content)(opts)(exts)(extensions) )

FC_REFLECT( graphene::chain::subject_vote_operation::fee_parameters_type, (fee_percent_of_vote_amount) )
FC_REFLECT( graphene::chain::subject_vote_operation, (fee)(voter)(subject_id)(template_vote) )

FC_REFLECT( graphene::chain::subject_event_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::subject_event_operation, (fee)(oper)(subject_id)(event)(options) )

