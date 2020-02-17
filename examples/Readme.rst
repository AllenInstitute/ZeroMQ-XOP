.. This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

Example programs interacting with the ZeroMQ XOP
------------------------------------------------
The result of each of these interactions between "Igor Pro Server" and the specified client should be Igor Pro displaying the help for ``GetDimLabel``.

Igor Pro Server
----------
Usage
~~~~~

- Start Igor Pro and execute the following code:

.. code-block:: igorpro

  zeromq_stop() // Stop any existing ZeroMQ operations
  zeromq_server_bind("tcp://127.0.0.1:5555") // Create a ZeroMQ server and begin listening
  zeromq_handler_start() // Prepare to handle incoming messages

C++ Client
----------
Compilation
~~~~~~~~~~~

- ``mkdir build``
- ``cd build``
- ``cmake -G "Visual Studio 14 2015" ..``
- ``cmake --build . --config Release``

Usage
~~~~~

- Now issue the following command from a terminal:

``zmq_xop_client.exe "tcp://127.0.0.1:5555" "{ \"version\" : 1, \"CallFunction\" : { \"name\" : \"ZeroMQ_ShowHelp\", \"params\" : [ \"GetDimLabel\" ] } }"``

Python Client
----------
Installation
~~~~~~~~~~~

- ``pip install pyzmq``

Usage
~~~~~

- Now issue the following commands from a Python shell:

.. code-block:: python

	import json # Built-in Python serialization library
	import zmq # The Python bindings for ZeroMQ
	context = zmq.Context() # Create a new ZeroMQ context
	socket = context.socket(zmq.REQ) # Make it act as a client
	socket.connect("tcp://127.0.0.1:5555") # Connect to the server at this IP and port
	func_name = "ZeroMQ_ShowHelp" # The name of the Igor function we want to call
	func_params = ["GetDimLabel"] # The arguments to that function
	# A command sent over a ZeroMQ socket to call a function
	command = {"version": 1, # Version of the protocol
        	   "CallFunction": # The action to do (call an Igor function)
               		{"name": func_name, # The name of the Igor function
                	 "params": func_params} # The arguments to that function
          	  }
	command_str = json.dumps(command) # JSONify it
	socket.send_string(command_str) # Send that command over the socket to Igor
