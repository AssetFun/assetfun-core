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

#include <graphene/delayed_node/delayed_node_plugin.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/app/api.hpp>

#include <fc/network/http/websocket.hpp>
#include <fc/rpc/websocket_api.hpp>
#include <fc/api.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/time.hpp>


namespace graphene { namespace delayed_node {
namespace bpo = boost::program_options;

namespace detail {
struct delayed_node_plugin_impl {
   std::string remote_endpoint;
   uint64_t backup_blockchain_per_block_amount; //每隔多少个区块备份一次区块链
   uint64_t block_num_for_recover;//恢复到指定高度的区块
   fc::http::websocket_client client;
   std::shared_ptr<fc::rpc::websocket_api_connection> client_connection;
   fc::api<graphene::app::database_api> database_api;
   boost::signals2::scoped_connection client_connection_closed;
   graphene::chain::block_id_type last_received_remote_head;
   graphene::chain::block_id_type last_processed_remote_head;
};
}

delayed_node_plugin::delayed_node_plugin()
   : my(new detail::delayed_node_plugin_impl)
{}

delayed_node_plugin::~delayed_node_plugin()
{}

void delayed_node_plugin::plugin_set_program_options(bpo::options_description& cli, bpo::options_description& cfg)
{
   cli.add_options()
         ("trusted-node", boost::program_options::value<std::string>()->required(), "RPC endpoint of a trusted validating node (required)")
         ;

   cli.add_options()
         ("backup-blockchain-per-block-amount", boost::program_options::value<uint64_t>(), "backup blockchain per how many blocks. Default value is 28800, or 1 day");

   cli.add_options()
         ("block-num-for-recover", boost::program_options::value<uint64_t>(), "specified block-num for recover");

   cfg.add(cli);
}

void delayed_node_plugin::connect()
{
   my->client_connection = std::make_shared<fc::rpc::websocket_api_connection>(*my->client.connect(my->remote_endpoint));
   my->database_api = my->client_connection->get_remote_api<graphene::app::database_api>(0);
   my->client_connection_closed = my->client_connection->closed.connect([this] {
      connection_failed();
   });
}

void delayed_node_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{
   ilog("delayed_node_plugin::plugin_initialize");
   my->remote_endpoint = "ws://" + options.at("trusted-node").as<std::string>();
   
   if( options.count("backup-blockchain-per-block-amount") )
   {
      my->backup_blockchain_per_block_amount = options["backup-blockchain-per-block-amount"].as<uint64_t>();
      ilog("backup-blockchain-per-block-amount: ${num}", ("num", my->backup_blockchain_per_block_amount));
   }
   else
   {
      my->backup_blockchain_per_block_amount = 28800;
      ilog("backup_blockchain_per_block_amount not set. Use default value 28800.");
   }

   if( options.count("block-num-for-recover") )
   {
      //如果config.ini的specied-recover-block-num字段的值不为0，表示同步区块只同步到该值指定的区块高度
      my->block_num_for_recover = options["block-num-for-recover"].as<uint64_t>();
      ilog("block-num-for-recover: ${num}", ("num", my->block_num_for_recover));
   }
   else
   {
      my->block_num_for_recover = 0;
      ilog("block-num-for-recover not set. Use default value 0.");
   }
   

}

void delayed_node_plugin::sync_with_trusted_node()
{
   auto& db = database();
   uint32_t synced_blocks = 0;
   uint32_t pass_count = 0;
   static bool recover_finish_flag = false;

   while( true )
   {
      graphene::chain::dynamic_global_property_object remote_dpo = my->database_api->get_dynamic_global_properties();
      if( remote_dpo.last_irreversible_block_num <= db.head_block_num() )
      {
         if( remote_dpo.last_irreversible_block_num < db.head_block_num() )
         {
            wlog( "Trusted node seems to be behind delayed node" );
         }
         if( synced_blocks > 1 )
         {
            ilog( "Delayed node finished syncing ${n} blocks in ${k} passes", ("n", synced_blocks)("k", pass_count) );
         }
         break;
      }
      pass_count++;
      while( remote_dpo.last_irreversible_block_num > db.head_block_num() && 
             (my->block_num_for_recover <= 0 || (my->block_num_for_recover > 0 && !recover_finish_flag) )
            ) 
      {
         fc::optional<graphene::chain::signed_block> block = my->database_api->get_block( db.head_block_num()+1 );
         FC_ASSERT(block, "Trusted node claims it has blocks it doesn't actually have.");
         ilog("Pushing block #${n}", ("n", block->block_num()));
         db.push_block(*block);
         synced_blocks++;

         // backup blockchain
         uint32_t block_num = block->block_num();

         if( block_num % my->backup_blockchain_per_block_amount == 0 )
         {
           db.flush_block();

           fc::time_point now = fc::time_point::now();
           fc::time_point_sec timestamp = now;
/*
           fc::microseconds interval = fc::hours(1);
            
           int64_t interval_seconds = interval.to_seconds();
           int64_t file_number = timestamp.sec_since_epoch() / interval_seconds;
           fc::time_point_sec start_time = fc::time_point_sec( (uint32_t)(file_number * interval_seconds) );
*/
           fc::time_point_sec start_time = fc::time_point_sec(timestamp.sec_since_epoch());
           std::string timestamp_string = start_time.to_non_delimited_iso_string();
           ilog("timestamp_string=${timestamp_string}", ("timestamp_string", timestamp_string));

           char cmd[1024];
           memset(cmd, 0 , sizeof(cmd));
           //sprintf(cmd, "cp -r /data/assetfun/delayed_node/delayed_node_data_dir/blockchain /data/assetfun/delayed_node/bak/blockchain%s_%d", timestamp_string.c_str(), block_num);
           sprintf(cmd, "cp -r ./delayed_node_data_dir/blockchain /data/assetfun/delayed_node/bak/blockchain_%s_%d", timestamp_string.c_str(), block_num);
           ilog("backup blockchain for delayed_node. cmd=${cmd}", ("cmd", cmd));
           system(cmd);
         }

         if( my->block_num_for_recover > 0 && block->block_num() >= my->block_num_for_recover)
         {
            ilog("recover has finished. block_num_for_recover=${num}", ("num", my->block_num_for_recover));
            recover_finish_flag = true;
         }
      }

      if( my->block_num_for_recover > 0 && recover_finish_flag)
      {
         break;
      }
   }
}

void delayed_node_plugin::mainloop()
{
   while( true )
   {
      try
      {
         fc::usleep( fc::microseconds( 296645 ) );  // wake up a little over 3Hz

         if( my->last_received_remote_head == my->last_processed_remote_head )
            continue;

         sync_with_trusted_node();
         my->last_processed_remote_head = my->last_received_remote_head;
      }
      catch( const fc::exception& e )
      {
         elog("Error during connection: ${e}", ("e", e.to_detail_string()));
      }
   }
}

void delayed_node_plugin::plugin_startup()
{
   fc::async([this]()
   {
      mainloop();
   });

   try
   {
      connect();
      my->database_api->set_block_applied_callback([this]( const fc::variant& block_id )
      {
         fc::from_variant( block_id, my->last_received_remote_head );
      } );
      return;
   }
   catch (const fc::exception& e)
   {
      elog("Error during connection: ${e}", ("e", e.to_detail_string()));
   }
   fc::async([this]{connection_failed();});
}

void delayed_node_plugin::connection_failed()
{
   elog("Connection to trusted node failed; retrying in 5 seconds...");
   fc::schedule([this]{connect();}, fc::time_point::now() + fc::seconds(5));
}

} }
