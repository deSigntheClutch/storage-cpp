https://redis.io/try/sandbox/

mkdir build && cd build
cmake ..
make

### Sorted-Set
* Create and enter build directory:
   `mkdir -p build && cd build`
* Generate build files: `cmake ..`
* Build everything: `make`
  * Or build specific targets:
```
make redis_demo     # sorted-set only
make publisher      # pub-sub publisher only                                                                                                                                                                   
make subscriber     # pub-sub subscriber only
```
* Run the programs:
```
# Sorted-set demo
./sorted-set/redis_demo

# Pub-sub (need 2 terminals)
./pub-sub/subscriber    # Terminal 1                                                                                                                                                                           
./pub-sub/publisher     # Terminal 2
```


Note: You need Redis server running first: `redis-server`
```
brew install redis                    # Install Redis
redis-server --daemonize yes         # Start Redis in background                                                                                                                                               
redis-cli ping                       # Test (should return PONG)

To stop Redis later:
redis-cli shutdown
```



