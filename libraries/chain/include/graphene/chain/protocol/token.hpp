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
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/protocol/asset.hpp>

#define SECONDS_OF_ONE_MONTH 2592000 //1个月按30天算，1 month = 2592000 seconds

namespace graphene { namespace chain { 

   struct token_rule
   {
      struct asset_symbol_amount
      {
        string     symbol; //资产(通证)缩写
        share_type amount; //资产(通证)数量，包含8位小数
      };
              
      //创建者设定的每份认购的quote_base比例，如每份认购 10 AFT = 20 积分
      struct each_buy_quote_base_ratio
      {
        asset_symbol_amount base;//核心资产
        asset_symbol_amount quote;//用户资产(或积分token)
      };

      //每个认购阶段的设置
      struct each_buy_phase_setting
      {
        time_point_sec            begin_time; //本认购阶段开始时间
        time_point_sec            end_time; // 本认购阶段结束时间
        each_buy_quote_base_ratio quote_base_ratio;
      };

      // 如果募集数量比例 >= token_template::buy_succeed_min_percent，但是没有达到100%，对于没有募集成功的那部分用户资产，发行人可以有如下处理方式
      enum not_buy_asset_handle_way
      {
        dispatch_to_buyer = 0, //剩余的没认购用户资产按比例分发给已经参与的人
        burn_asset        = 1 //剩余的没认购用户资产直接销毁
      };
   };


   /*
    * token tempalte: 创建众筹模板
    * 当need_raising=true, plan_buy_total=0时，为非法的配置
    * 当need_raising=true, plan_buy_total>0时，表示需要募集，计划用于募集的用户资产(通证)数量为plan_buy_total，通证不认购部分分期发放
    * 当need_raising=false, plan_buy_total=0时，表示不需要募集，直接创建通证，所有的通证一次性直接给发行人(issuer_reserved_asset_frozen_months直接置为0)，目的是为了兼容公测环境的旧数据
    * 当need_raising=false, plan_buy_total=max_supply时，表示不需要募集，直接创建通证，所有的通证一次性直接给发行人(issuer_reserved_asset_frozen_months直接置为0)
    * 当need_raising=false, plan_buy_total>0时，表示不需要募集，直接给发行人plan_buy_total数量的通证(可视为在其它平台上进行募集)，而且也不抵押，通证剩余部分分期发放(issuer_reserved_asset_frozen_months>=0,为0时表示创建通证后马上发放)
    */
   struct token_template {
      string                  asset_name; //用户资产(或通证token)名称/通证名称
      string                  asset_symbol; //用户资产(或通证token)缩写/通证简称
      string                  type; //通证(众筹项目)类型
      string                  subtype = ""; //通证(众筹项目)子类型，可以没有子类型
      string                  logo_url; // 用户资产logo链接
      string                  max_supply; //用户资产总(最大)发行量，包含8位小数
      bool                    need_raising = true; //是否需要在AssetFun平台上募集。
      string                  plan_buy_total = "0"; //当需要募集时，表示用户资产计划募集用户资产数量，包含8位小数。用户资产募集数量 <= 用户资产总发行量。当不需要募集时，表示创建通证时直接给发行人的通证数量
      uint8_t                 buy_succeed_min_percent; //用户资产募集成功的最少比例(基数为plan_buy_total)，[1-100], 2-3位整数，最大100， 如1234表示12.34%，如值40*GRAPHENE_1_PERCENT表示40%
      std::map<string, token_rule::each_buy_phase_setting>  buy_phases; //各个募集阶段的设置，string为"1", "2", 表示认购阶段1，认购阶段2
      uint8_t                 not_buy_asset_handle = 0; //没有募集成功的那部分用户资产的处理方式
      asset                   guaranty_core_asset_amount = asset(0, asset_id_type()); //发行人核心资产(AFT)抵押数量，包含8位小数
      uint8_t                 guaranty_core_asset_months = 0; //发行人核心资产(AFT)抵押时长，按月算，1表示1个月，2表示2个月，范围1-36
      uint8_t                 issuer_reserved_asset_frozen_months = 0; //发行人预留的用户资产(未募集的用户资产部分)解冻时长，按月算，1表示1个月，2表示2个月，范围1-36
      vector<string>          whitelist = {}; //用户资产募集白名单，空表示任何人均可认购
      string                  brief; //通证(众筹项目)摘要
      string                  description; //通证(众筹项目)描述
      map<string, string>     customized_attributes; //自定义属性，<属性名,  属性>

      /// Perform internal consistency checks.
      /// @throws fc::exception if any check fails
      void validate()const;
   };


   struct encrypt_data
   {
      public_key_type from;
      public_key_type to;
      /**
       * 64 bit nonce format:
       * [  8 bits | 56 bits   ]
       * [ entropy | timestamp ]
       * Timestamp is number of microseconds since the epoch
       * Entropy is a byte taken from the hash of a new private key
       *
       * This format is not mandated or verified; it is chosen to ensure uniqueness of key-IV pairs only. This should
       * be unique with high probability as long as the generating host has a high-resolution clock OR a strong source
       * of entropy for generating private keys.
       */
      uint64_t nonce = 0;
      /**
       * This field contains the AES encrypted packed @ref memo_message
       */
      vector<char> message;
   };

   /*
    * 认购参数模板
    */
   struct token_buy_template
   {
      enum token_buy_phase
      {
        buy_phase1 = 1, //认购第1阶段
        buy_phase2 = 2  //认购第2阶段
      };

      uint8_t                    phase; //认购哪个阶段
      uint64_t                   buy_quantity; // 认购份数，认购份数没有限制
      optional<encrypt_data>     goods_receiver_contact_info;    // 众筹发放的实物的接收者联系信息
      //optional<string>     goods_receiver_contact_info;    // 众筹发放的实物的接收者联系信息

   };

   /**
    * 发起众筹项目 
    */
   struct token_publish_operation : public base_operation
   {
      struct fee_parameters_type
   	  {
        uint64_t fee = 5000 * GRAPHENE_BLOCKCHAIN_PRECISION;       //基础费用5000个AFT
      };

   	  asset           		  fee;

   	  // This account must sign and pay the fee for this operation. Later, this account may update the asset
      account_id_type       issuer; //用户资产发行人/众筹项目发起人

      token_template      template_parameter;

      // token profile setting
      optional<map<string, string>>  exts; // extend_options

      extensions_type       extensions;


      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee( const fee_parameters_type& k )const;
   };

   /**
    * 认购
    */
   struct token_buy_operation : public base_operation
   {
      struct fee_parameters_type
      {
         uint64_t fee       = 0.2 * GRAPHENE_BLOCKCHAIN_PRECISION; //基础费用0.2个AFT
      };

      asset                  fee;
      account_id_type        buyer; //认购者账号id
      token_id_type        token_id;

      // buy template
      token_buy_template   template_parameter;


      uint64_t get_buy_quantity()const 
      {
         return template_parameter.buy_quantity;
      }

      account_id_type fee_payer()const { return buyer; }
      void            validate()const;
      share_type      calculate_fee( const fee_parameters_type& k )const;

      optional<map<string, string>>  exts; // extend_options
      extensions_type       extensions;
   };

   /**
    * @class  token_event_operation
    * @brief  the token buy instrument update token_object status for prediction, 
    *         and handle event
    * @ingroup operations
    * 
    * ...
    */
   struct token_event_operation : public base_operation
   {
      struct fee_parameters_type
      {
         uint64_t fee       = 0;
      };

      asset             fee;
      account_id_type   oper; // witness or committee or system ?
      token_id_type   token_id;
      string            event; //
      
      variant           options;

      //account_id_type fee_payer()const { return oper; }
      account_id_type fee_payer()const 
      { 
        return GRAPHENE_COMMITTEE_ACCOUNT;
      }

      void            validate()const;
      share_type      calculate_fee( const fee_parameters_type& k )const;
   };

} } // graphene::chain


FC_REFLECT_ENUM(graphene::chain::token_rule::not_buy_asset_handle_way, (dispatch_to_buyer)(burn_asset))
FC_REFLECT(graphene::chain::token_rule::asset_symbol_amount, (symbol)(amount))
FC_REFLECT(graphene::chain::token_rule::each_buy_quote_base_ratio, (base)(quote))
FC_REFLECT(graphene::chain::token_rule::each_buy_phase_setting, (begin_time)(end_time)(quote_base_ratio))


FC_REFLECT(graphene::chain::token_template, (asset_name)(asset_symbol)(type)(subtype)(logo_url)(max_supply)(need_raising)(plan_buy_total)(buy_succeed_min_percent)(buy_phases)
                            (not_buy_asset_handle)(guaranty_core_asset_amount)(guaranty_core_asset_months)(issuer_reserved_asset_frozen_months)(whitelist)(brief)(description)(customized_attributes))
//FC_REFLECT(graphene::chain::extend_options, (opt))
//FC_REFLECT(graphene::chain::query_for_token_publish_paramter, (create_time)(prediction_interval)(vote_begin_time)(vote_end_time)(settle_time)(create_fee) )
FC_REFLECT(graphene::chain::encrypt_data, (from)(to)(nonce)(message))
FC_REFLECT(graphene::chain::token_buy_template, (phase)(buy_quantity)(goods_receiver_contact_info))

FC_REFLECT( graphene::chain::token_publish_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::token_publish_operation, (fee)(issuer)(template_parameter)(exts)(extensions) )

FC_REFLECT( graphene::chain::token_buy_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::token_buy_operation, (fee)(buyer)(token_id)(template_parameter)(exts)(extensions) )

FC_REFLECT( graphene::chain::token_event_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::token_event_operation, (fee)(oper)(token_id)(event)(options) )

