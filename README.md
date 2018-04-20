AssetFun Core
==============
* [Getting Started](#getting-started)
* [Support](#support)
* [License](#license)

AssetFun Core is the AssetFun blockchain implementation and command-line interface.

Visit [assetfun.org](https://assetfun.org/) to learn about AssetFun.

**NOTE:** The official AssetFun git repository location, default branch, and submodule remotes were recently changed. Existing
repositories can be updated with the following steps:

    git remote set-url origin https://github.com/assetfun/assetfun-core.git
    git checkout master
    git remote set-head origin --auto
    git pull
    git submodule sync --recursive
    git submodule update --init --recursive

Getting Started
---------------
Build instructions and additional documentation are available in the
[wiki](https://github.com/assetfun/assetfun-core/wiki).

We recommend building on Ubuntu 16.04 LTS, and the build dependencies may be installed with:

    sudo apt-get update
    sudo apt-get install autoconf cmake git libboost-all-dev libssl-dev

To build after all dependencies are installed:

    git clone https://github.com/assetfun/assetfun-core.git
    cd assetfun-core
    git checkout <LATEST_RELEASE_TAG>
    git submodule update --init --recursive
    cmake -DCMAKE_BUILD_TYPE=release .
    make witness_node delayed_node cli_wallet

**NOTE:** AssetFun requires an [OpenSSL](https://www.openssl.org/) version in the 1.0.x series. OpenSSL 1.1.0 and newer are NOT supported. If your system OpenSSL version is newer, then you will need to manually provide an older version of OpenSSL and specify it to CMake using `-DOPENSSL_INCLUDE_DIR`, `-DOPENSSL_SSL_LIBRARY`, and `-DOPENSSL_CRYPTO_LIBRARY`.

**NOTE:** AssetFun requires a [Boost](http://www.boost.org/) version in the range [1.57, 1.60]. Versions earlier than
1.57 or newer than 1.60 are NOT supported. If your system Boost version is newer, then you will need to manually build
an older version of Boost and specify it to CMake using `DBOOST_ROOT`.

After building, the witness node can be launched with:

    ./programs/witness_node/witness_node

The node will automatically create a data directory including a config file. It may take several hours to fully synchronize
the blockchain. After syncing, you can exit the node using Ctrl+C and setup the command-line wallet by editing
`witness_node_data_dir/config.ini` as follows:

    rpc-endpoint = 127.0.0.1:8090

After starting the witness node again, in a separate terminal you can run:

    ./programs/cli_wallet/cli_wallet

Set your inital password:

    >>> set_password <PASSWORD>
    >>> unlock <PASSWORD>

To import your initial balance:

    >>> import_balance <ACCOUNT NAME> [<WIF_KEY>] true

If you send private keys over this connection, `rpc-endpoint` should be bound to localhost for security.

Use `help` to see all available wallet commands.


After starting the witness node, the delayed node can be launched with:

    ./programs/delayed_node/delayed_node


Support
-------
Technical support is available in the [AssetFun technical support](https://assetfun.org).

AssetFun Core bugs can be reported directly to the [issue tracker](https://github.com/assetfun/assetfun-core/issues).

 
License
-------
AssetFun Core is under the GNU General Public License v3. See [LICENSE](https://github.com/assetfun/assetfun-core/blob/master/LICENSE)
for more information.
