/**
 * The load balancer is responsible for creating and managing the threads that will handle the packet processing.
 * See toeplitz.c for the hashing function used to determine which thread to send the packet to.
 * 
 * The load balancer will also be responsible for creating the thread pool for the packet processing.
 * 
 * So we need a thread pool for the packet processing, and a thread for the switching.
 * To handle message passing between the threads, we will use a lock free ring buffer for packet queues.
 * 
 */