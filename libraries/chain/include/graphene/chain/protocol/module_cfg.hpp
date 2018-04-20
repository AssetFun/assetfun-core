/*
 * Copyright (c) 2017 AssetFun, Inc., and contributors.
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
#include <graphene/chain/protocol/memo.hpp>
#include <graphene/chain/config.hpp>

static const std::string COIN_MODULE_NAME = "COIN";
static const std::string TOKEN_MODULE_NAME = "TOKEN"; //TOKEN参数配置模块
static const std::string WORD_MODULE_NAME  = "WORD"; //敏感词参数配置模块

namespace graphene { namespace chain {

   /**
    * @brief config module
    * @ingroup operations
    *
    */
  static const uint8_t module_cfg_op_update = 1;
  static const uint8_t module_cfg_op_insert = 2;
  static const uint8_t module_cfg_op_delete = 3;

   struct module_cfg_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset                  fee; ///< paid for by account_id_type
      account_id_type        proposer;
      string                 module_name; ///< module to config
      variant_object         cfg_value;
      uint8_t                op_type;  //1 - update; 2 - insert; 3 - delete
      extensions_type        extensions;

      //TODO change fee payer
      account_id_type fee_payer()const 
      { 
        if(module_name == COIN_MODULE_NAME)
          return GRAPHENE_WITNESS_ACCOUNT;
        else
          return GRAPHENE_COMMITTEE_ACCOUNT;
      }
      void            validate()const;
   };

} } // graphene::chain




FC_REFLECT( graphene::chain::module_cfg_operation::fee_parameters_type, (fee) )

FC_REFLECT( graphene::chain::module_cfg_operation,
            (fee)(proposer)(module_name)(cfg_value)(op_type)(extensions) );

