/// @file interface.h Available functions in the ZeroMQ XOP
///
/// This file is part of the `ZeroMQ-XOP` project and licensed under
/// BSD-3-Clause.
///
/// - All functions returning a variable always return zero
/// - Errors are signalled via RTE, use
/// @code
/// zeromq_bind("blah"); err = GetRTError(1)
/// @endcode
/// for programmatically handling the error.
/// The error values are defined at @ref ZeroMQErrorCodes.
/// - The XOP applies the following settings:
///   - Message pattern: DEALER/ROUTER
///   - DEALER/ROUTER: ZMQ_SNDTIMEO = 0
///   - DEALER/ROUTER: ZMQ_RCVTIMEO = 1
///   - DEALER/ROUTER: ZMQ_LINGER = 0
///   - ROUTER: ZMQ_ROUTER_MANDATORY = 1
///   - ROUTER: ZMQ_MAXMSGSIZE = 1024 byte
///   - DEALER: ZMQ_IDENTITY non-empty string
/// - For compatibility reasons with the REQ socket, DEALER sockets have to
///   always send two frames: An empty one and one with the payload.

/// @brief Stop all listening binds and disconnect outgoing connections, stops
/// also the message handler.
///        Internally closes also the sockets.
///
///        Will be implicitly called on Igor close.
THREADSAFE variable zeromq_stop();

/// @brief Allow to adjust various runtime features.
///
/// The flags can be or'ed.
///
/// @param flags One of @ref ZeroMQSetFlags
THREADSAFE variable zeromq_set(variable flags);

/// @brief Start listening on the given TCP port
///
/// @param localPoint transport protocol and address, something like
///                   `tcp://127.0.0.1:5670` should be used, the TCP/IP port
///                   must be > 1024 and unused, 5670 is a good default choice
///                   [standard port for FileMQ]
THREADSAFE variable zeromq_server_bind(string localPoint);

/// Send a messsage as the `ROUTER` socket
///
/// @param identity client identifier
/// @param msg      Message to send
THREADSAFE variable zeromq_server_send(string identity, string msg);

/// Receive a message
///
/// Implemented using a blocking wait albeit abortable from within Igor Pro.
///
/// @param[out] identity client identifier, required for sending a message back
/// via zeromq_server_send()
///
/// @return received message
THREADSAFE string zeromq_server_recv(string *identity);

/// Send a messsage as the `DEALER` socket
///
/// @param msg      Message to send
THREADSAFE variable zeromq_client_send(string msg);

/// @brief Connect to a ZMQ server, e.g. a different Igor instance running as a
/// server
///
/// @param remotePoint Protocol and address of the server, usually something
/// like `tcp://127.0.0.1:5670`
THREADSAFE variable zeromq_client_connect(string remotePoint);

/// Receive a message as the `DEALER` socket
///
/// Implemented using a blocking wait albeit abortable from within Igor Pro.
///
/// @return received message
THREADSAFE string zeromq_client_recv();

/// @name Publishers and Subscribers
///
/// @{

/// @brief Start listening on the given TCP port as ZMQ_PUB socket
///
/// @param localPoint transport protocol and address, something like
///                   `tcp://127.0.0.1:5670` should be used, the TCP/IP port
///                   must be > 1024 and unused, 5670 is a good default choice
///                   [standard port for FileMQ]
THREADSAFE variable zeromq_pub_bind(string localPoint);

/// @brief Publish the given message to all connected subscribers
///
/// Message filtering happens on the publisher side, around ZMQ_SNDHWM messages
/// (by default 1000) will be kept.
THREADSAFE variable zeromq_pub_send(string filter, string msg);

/// @brief Multipart variant of zeromq_pub_send
THREADSAFE variable zeromq_pub_send_multi(WAVEWAVE payload);

/// @brief Connect to a ZMQ_PUB socket as ZMQ_SUB
///
/// @param remotePoint Protocol and address of the server, usually something
/// like `tcp://127.0.0.1:5670`
///
/// By default there are no subscriptions, be sure to call zeromq_sub_add_filter
/// at least once.
THREADSAFE variable zeromq_sub_connect(string remotePoint);

/// @brief Add a message filter
///
/// @param filter message type to add a subscription for. Using an empty string
/// will subscribe to all messages.
///
/// Multiple filters are or'ed, which means a message must match at least one
/// filter.
///
/// The filter types are binary compared.
THREADSAFE variable zeromq_sub_add_filter(string filter);

/// @brief Removes a previously added message filter
///
/// @param filter message type to remove a subscription for. Use an empty string
/// to unsubscribe from all messages.
THREADSAFE variable zeromq_sub_remove_filter(string filter);

/// @brief Receive subscribed messages
THREADSAFE string zeromq_sub_recv(string *filter);
/// @}

/// @brief Receive subscribed messages (multipart)
THREADSAFE variable zeromq_sub_recv_multi(WAVEWAVE payload);
/// @}

/// @name Message handler
///
/// The XOP implements a threaded message handler. This message handler can be
/// started and stopped with the following commands.
/// @{
variable zeromq_handler_start();

/// Will be implicitly called on Igor close.
variable zeromq_handler_stop();
/// @}

/// Set logging template
///
/// Set the JSON text used as template for the JSONL log file.
///
/// This function does not enable logging, use
/// `zeromq_set(ZeroMQ_SET_FLAGS_LOGGING)` for that.
///
/// Requirement:
/// - Valid JSON text
/// - Top-level entity must be a JSON object and does not have reserved keys
THREADSAFE variable zeromq_set_logging_template(string jsonString);

/// Set the name of the interceptor function, the prototype must match
/// ZeroMQ_Interceptor_Proto()
THREADSAFE variable zeromq_set_interceptor_func(string funcName);

/// @cond DOXYGEN_IGNORES_THIS
/// @name Functions used for testing and debugging
/// @{
THREADSAFE string zeromq_test_callfunction(string msg);
THREADSAFE string zeromq_test_serializeWave(WAVE wv);

/// Regression test for a race between HeartbeatPublisher::Start() and
/// Stop(): runs `iterations` back-to-back Start()/Stop() cycles, on a
/// detached worker thread, with the heartbeat publisher's normal
/// steady-state (started) status restored on completion.
///
/// If the race is present, a Stop() call can block forever, so this waits
/// for the worker thread to finish only up to an internal timeout and
/// throws ZMQ_HEARTBEAT_STARTSTOP_DEADLOCK instead of hanging Igor Pro
/// itself when that happens.
///
/// @param iterations number of Start()/Stop() cycles to perform, must be >= 1
THREADSAFE variable zeromq_test_hb_startstop(variable iterations);

/// Regression test for the IdleGuard fix in XOPEntry's IDLE case: directly
/// exercises IdleGuard the same way XOPEntry does, throwing from within its
/// scope, and checks that idleInProgress was reset by the guard's
/// destructor despite the exception rather than staying stuck at `true`
/// (which would permanently disable all further IDLE processing).
///
/// Not threadsafe: idleInProgress is plain (non-atomic) main-thread-only
/// state, exactly like the real IDLE dispatch in XOPEntry.
variable zeromq_test_idleguard();

/// Regression test for ConcurrentQueue::apply_to_all's queue-swap fix: the
/// functor passed to apply_to_all must be able to push() back into the same
/// queue, on the same thread, without deadlocking. Runs entirely on a local
/// ConcurrentQueue, on a detached worker thread with an internal timeout, so
/// a reintroduced deadlock surfaces as a thrown ZMQ_QUEUE_APPLY_DEADLOCK
/// error instead of hanging Igor Pro itself.
THREADSAFE variable zeromq_test_queueswap();

/// Regression test for the GET_SOCKET/SocketWithMutex fetch-then-relock race
/// (see GlobalData::GetOrCreateSocket, SocketWithMutex.h): one thread
/// repeatedly closes and rebinds the Publisher socket via
/// GlobalData::CloseConnections() (the same path zeromq_stop() uses) while
/// another concurrently calls ZeroMQPublisherSend() -- the same function
/// HeartbeatPublisher's always-running background thread uses -- in a tight
/// loop, to get many race attempts quickly.
///
/// Before the fix, GET_SOCKET fetched the socket pointer and only
/// afterward re-acquired the mutex, leaving a window where the closing
/// thread could invalidate the socket a waiting thread was about to use;
/// the sending thread could then hang inside libzmq while still holding
/// the Publisher mutex, which the closing thread's next iteration would
/// then block on forever -- the actual production freeze this reproduces.
/// Throws ZMQ_SOCKET_CLOSE_RACE_DEADLOCK instead of hanging Igor Pro itself
/// if that happens.
THREADSAFE variable zeromq_test_socketclose_race();

/// Regression test for MessageHandlerPauseGuard (see
/// MessageHandlerPauseGuard.h): establishes a dummy Server bind, starts the
/// message handler, and checks that constructing the guard stops it
/// (IsRunning() becomes false), that destroying the guard restarts it
/// (IsRunning() becomes true again), and that a disabled guard
/// (enable = false) is a true no-op. Restores a stopped, unbound baseline
/// on completion.
THREADSAFE variable zeromq_test_msghandler_pause();
/// @}
/// @endcond
