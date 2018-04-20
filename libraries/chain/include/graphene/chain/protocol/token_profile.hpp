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


namespace graphene { namespace chain {

	//用户资产(或通证token)名称/通证名称
	struct asset_name_length
	{
		uint64_t	min = 3;
		uint64_t	max = 32;
	};

	//用户资产(或通证token)缩写/通证简称
	struct asset_symbol_length
	{
		uint64_t	min = 3;
		uint64_t	max = 6;
	};

	//通证(众筹项目)摘要
	struct token_brief_length
	{
		uint64_t	min = 0;
		uint64_t	max = 120;
	};

	//通证(众筹项目)描述
	struct token_description_length
	{
		uint64_t	min = 0;
		uint64_t	max = 10240;
	};

	//用户资产募集天数(项目开放的时长)， 按天算，7-90天
	struct token_collect_days
	{
		uint64_t	min = 7; // 7天
		uint64_t	max = 90; // 90天
	};

	//发行人核心资产(AFT)抵押时长，按月算，1表示1个月，2表示2个月，范围1-60。到期后一次性解冻返还给发行人
	struct token_guaranty_core_asset_months
	{
		uint64_t	min = 1; // 1个月
		uint64_t	max = 60; // 60个月
	};

	//每份认购需要花费的核心资产金额范围: 1 - 100万AFT
	struct token_each_buy_core_asset_range
	{
	  asset min = {100000000,asset_id_type()};
	  asset max = {100000000000000,asset_id_type()};
	};

    //发行人预留的用户资产(未募集的用户资产部分)解冻时长，按月算，1表示1个月，2表示2个月，范围1-60
	struct token_issuer_reserved_asset_frozen_months
	{
		uint64_t	min = 1; // 1个月
		uint64_t	max = 60; // 60个月
	};

	struct token_change_profie
	{
		map<string, string>                            token_types; // 通证类型，<token_type_en, token_type_cn>
		map<string, map<string, string>>               token_subtypes; // 通证子类型, <token_type_en, map<token_subtype_en, token_subtype_cn>>
		asset_name_length							   name_length;
		asset_symbol_length							   symbol_length;
		token_brief_length							   brief_length;
		token_description_length					   description_length;
		uint64_t									   max_days_between_create_and_phase1_begin;
		token_collect_days                             collect_days; //用户资产募集天数(项目开放的时长)， 7-90天
		token_guaranty_core_asset_months               guaranty_core_asset_months; //发行人核心资产(AFT)抵押时长，按月算
		token_each_buy_core_asset_range                each_buy_core_asset_range; //每份认购需要花费的核心资产金额范围: 1 - 100万AFT
		token_issuer_reserved_asset_frozen_months      issuer_reserved_asset_frozen_months; //发行人预留的用户资产(未募集的用户资产部分)解冻时长，按月算
		uint64_t					                   buy_max_times = 10; //同一众筹一个用户最多可认购10次
		uint64_t					                   whitelist_max_size = 200; //用户资产募集白名单最大长度
		vector<string>                                 reserved_asset_names;//系统保留的用户资产(或通证token)名称/通证名称
		vector<string>                                 reserved_asset_symbols;//系统保留的用户资产(或通证token)缩写/通证简称
	};

} } // graphene::chain


FC_REFLECT( graphene::chain::asset_name_length, (min)(max) )
FC_REFLECT( graphene::chain::asset_symbol_length, (min)(max) )
FC_REFLECT( graphene::chain::token_brief_length, (min)(max) )
FC_REFLECT( graphene::chain::token_description_length, (min)(max) )
FC_REFLECT( graphene::chain::token_collect_days, (min)(max) )
FC_REFLECT( graphene::chain::token_guaranty_core_asset_months, (min)(max) )
FC_REFLECT( graphene::chain::token_each_buy_core_asset_range, (min)(max) )
FC_REFLECT( graphene::chain::token_issuer_reserved_asset_frozen_months, (min)(max) )
FC_REFLECT( graphene::chain::token_change_profie, 
	(token_types)(token_subtypes)(name_length)(symbol_length)(brief_length)(description_length)(max_days_between_create_and_phase1_begin)
	(collect_days)(guaranty_core_asset_months)(each_buy_core_asset_range)(issuer_reserved_asset_frozen_months)(buy_max_times)(reserved_asset_names)(reserved_asset_symbols) )

