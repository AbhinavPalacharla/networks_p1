- Have client send packet to server to know when it is exiting so server can stop sending it messages

- Before displaying text, the client application should print many backspace characters ('\b' in C)
  to erase the prompt and anything the local user has typed. After the text is printed, the client
  should redisplay the prompt and any user input. This means the client will need to keep track of
  the user's input as they are typing it.

- NEED TO HANDLE INPUT VALIDATION
- HANDLE IF USER LEAVES THEIR CURRENT CHANNEL, NEED TO CHANGE DEFAULT CHANNEL TO SOMETHING ELSE

- After confirmation recieved that user joined channel, set their current channel to that one
  (for sending purposes). Make this a seperate function called handle_recieved_packet() or something
  This will not be handled in this function

- Need to handle input validation on client for things like giving channel name longer than allowed channel name, username longer than allowed, etc.

- If a user switches channels do we need to check and see if the channel exists before switching or if they switch to a channel that doesn't exist and send a message should we just create the channel and then send the message

  - Can check and see if user has joined channel before switching to it

- Error checking for if executable is ran with proper arguments
