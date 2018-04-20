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


namespace graphene { namespace chain {


	struct subject_delay_settle
	{
		//time_point_sec 	fixed = 0; // it is unuseful currently
		uint64_t		delay = 600; // 600 seconds
	};

	struct subject_vote_amount_range//投票者投票金额范围 1个，最多100万AFT
	{
		asset min={100000000,asset_id_type()};
		asset max = {100000000000000,asset_id_type()};
	};

	struct subject_prediction_duration_limit
	{
		uint64_t	less = 3600; // 1 hour
		uint64_t	more = 157680000; // 5 years = 157680000 seconds
	};

/*	struct subject_create_fee
	{
		uint64_t  	min_amount = 1; // 1 AFT
		uint32_t	fee_per_hour = 5; // 5 AFT
		string 		asset_id = "1.3.0"; // AFT
	};
*/
	struct subject_creator_shares
	{
		uint64_t	trade_fee_percent = 30*GRAPHENE_1_PERCENT; // 投票手续费的30%归主题创建者
		uint64_t	win_fund_percent = 5*GRAPHENE_1_PERCENT; // 如果创建者赢了，可额外获得奖励资金池的5%
	};

	struct subject_creator_vote //创建者至少要押注50个AFT，最多100万AFT
	{
		asset	min = {5000000000, asset_id_type()};
		asset	max = {100000000000000, asset_id_type()}; 
	};	

	struct subject_change_profie
	{
		uint64_t					delay_for_judge = 180; //由于从网站取喂价存在一定的延时，所以裁决时间比预测结束时间要延迟一段时间，现为180秒
		subject_delay_settle 		delay_settle;
		uint64_t					delay_for_restore = 86400; // 预测结束时间多久后(现为24小时, 即86400秒)后如仍未自动结算，系统将做自动清退处理，主题的状态改为restore
		uint64_t 					vote_duration_percent = 40*GRAPHENE_1_PERCENT; //投票周期为整个预测主题周期的35%
		subject_vote_amount_range 	vote_amount_range;
		uint64_t					vote_max_times = 5; //同一预测主题一个用户最多可投注5次
		subject_prediction_duration_limit prediction_duration_limit;
		//subject_create_fee			create_fee; 
		subject_creator_shares		creator_shares; 
		subject_creator_vote 		creator_vote;
	};


} } // graphene::chain


FC_REFLECT( graphene::chain::subject_delay_settle, (delay) )
FC_REFLECT( graphene::chain::subject_vote_amount_range, (min)(max) )
FC_REFLECT( graphene::chain::subject_prediction_duration_limit, (less)(more) )
//FC_REFLECT( graphene::chain::subject_create_fee, (min_amount)(fee_per_hour)(asset_id) )
FC_REFLECT( graphene::chain::subject_creator_shares, (trade_fee_percent)(win_fund_percent) )
FC_REFLECT( graphene::chain::subject_creator_vote, (min)(max) )
FC_REFLECT( graphene::chain::subject_change_profie, 
	(delay_for_judge)(delay_settle)(delay_for_restore)(vote_duration_percent)(vote_amount_range)(vote_max_times)(prediction_duration_limit)(creator_shares)(creator_vote) )

