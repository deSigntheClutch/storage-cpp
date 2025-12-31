
# Get cluster info
redis-cli -c CLUSTER NODES

# Check which node a channel maps to
redis-cli -c CLUSTER KEYSLOT channel_name

# List active channels on current node
redis-cli -c PUBSUB CHANNELS

# Check subscribers count for a channel
redis-cli -c PUBSUB NUMSUB channel_name


To check all nodes in cluster:

# Connect to each node and check channels
redis-cli -c -p 7000 PUBSUB CHANNELS                                                                                                                                                                             
redis-cli -c -p 7001 PUBSUB CHANNELS                                                                                                                                                                             
redis-cli -c -p 7002 PUBSUB CHANNELS


Monitor pub-sub activity:
bash
redis-cli MONITOR  # Shows all commands including pub-sub
redis-cli PSUBSCRIBE '*'  # Subscribe to all channels to see activity  