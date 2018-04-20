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
#include <graphene/chain/protocol/ext.hpp>

#define INVALID_FEED_PRICE -100000000

namespace graphene { namespace chain {

   /**
    * @brief Update the set of feed-producing accounts for a BitAsset
    * @ingroup operations
    *
    * BitAssets have price feeds selected by taking the median values of recommendations from a set of feed producers.
    * This operation is used to specify which accounts may produce feeds for a given BitAsset.
    *
    * @pre @ref issuer MUST be an existing account, and MUST match asset_object::issuer on @ref asset_to_update
    * @pre @ref issuer MUST NOT be the committee account
    * @pre @ref asset_to_update MUST be a BitAsset, i.e. @ref asset_object::is_market_issued() returns true
    * @pre @ref fee MUST be nonnegative, and @ref issuer MUST have a sufficient balance to pay it
    * @pre Cardinality of @ref new_feed_producers MUST NOT exceed @ref chain_parameters::maximum_asset_feed_publishers
    * @post @ref asset_to_update will have a set of feed producers matching @ref new_feed_producers
    * @post All valid feeds supplied by feed producers in @ref new_feed_producers, which were already feed producers
    * prior to execution of this operation, will be preserved
    */
   struct coin_update_feed_producers_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 500 * GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset           fee;
      account_id_type publisher;
      coin_id_type   coin_to_update;

      flat_set<account_id_type> new_feed_producers;
      extensions_type           extensions;

      account_id_type fee_payer()const { return publisher; }
      void            validate()const;
   };

//   enum coin_base
//   {
//	   CNY = 0,
//	   USD
//   };
   struct coin_price
   {
	   static const uint16_t PRECISION = 8;

	   share_type price = INVALID_FEED_PRICE;  //喂价网关以这个值做无效价格，这里也用这个数值
     string platform_quote_base; //e.g. "1000001:BTC/USD"
	   string src;//such as www.okcoin.cn

	   bool operator < (const coin_price& rht) const
	   {
		   return this->price < rht.price;
	   }

     coin_price()
     {
        price = INVALID_FEED_PRICE;
        platform_quote_base = "";
     }

     coin_price(string platform_id_, string quote_base_, share_type price_=INVALID_FEED_PRICE)
     {
        price = price_;
        platform_quote_base = platform_id_ + ":" + quote_base_;
     }

	   bool valid() const {return price != INVALID_FEED_PRICE;}
   };

   struct platform_qbase
   {
    string platform_id;
    string quote_base;
   };

   /**
    * @brief Publish price feeds for market-issued assets
    * @ingroup operations
    *
    * Price feed providers use this operation to publish their price feeds for market-issued assets. A price feed is
    * used to tune the market for a particular market-issued asset. For each value in the feed, the median across all
    * committee_member feeds for that asset is calculated and the market for the asset is configured with the median of that
    * value.
    *
    * The feed in the operation contains three prices: a call price limit, a short price limit, and a settlement price.
    * The call limit price is structured as (collateral asset) / (debt asset) and the short limit price is structured
    * as (asset for sale) / (collateral asset). Note that the asset IDs are opposite to eachother, so if we're
    * publishing a feed for USD, the call limit price will be CORE/USD and the short limit price will be USD/CORE. The
    * settlement price may be flipped either direction, as long as it is a ratio between the market-issued asset and
    * its collateral.
    */
   struct coin_feed_price_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 100; };

      struct ext
      {
         optional< void_t >            null_ext;
      };

      asset                  fee; ///< paid for by publisher
      account_id_type        publisher;
      coin_id_type          coin_id; ///< coin for which the feed is published
      //<time in second, coin price>
      //map<uint32_t, coin_price> prices;
      string platform_id;
      string quote_base;
      map<uint32_t, share_type> prices;
      bool reset_price = false;
      extension< ext >        extensions;

      account_id_type fee_payer()const { return publisher; }
      void            validate()const;
   };



} } // graphene::chain




FC_REFLECT( graphene::chain::coin_update_feed_producers_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::coin_feed_price_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::coin_feed_price_operation::ext, (null_ext) )

FC_REFLECT( graphene::chain::coin_price,
            (price)(platform_quote_base)(src)
		  )

FC_REFLECT( graphene::chain::platform_qbase,
            (platform_id)(quote_base)
      )

FC_REFLECT( graphene::chain::coin_update_feed_producers_operation,
            (fee)(publisher)(coin_to_update)(new_feed_producers)(extensions)
          )
FC_REFLECT( graphene::chain::coin_feed_price_operation,
            (fee)(publisher)(coin_id)(platform_id)(quote_base)(prices)(reset_price)(extensions) );

