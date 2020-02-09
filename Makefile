json:
	wget raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -O ./lib/json.hpp

server:	
	rm -rf ./lib/server ./lib/brynet
	git clone https://github.com/IronsDu/brynet.git ./lib/brynet
	cp -r ./lib/brynet/include/brynet/ ./lib/server/