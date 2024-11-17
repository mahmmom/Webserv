#!/bin/bash

# Set the Content-Type header for plain text
echo "Content-type: text/html"
echo ""

# Start HTML content
echo "<html>"
echo "<head><title>Testing CGI Process Timeout</title></head>"
echo "<body>"
echo "<h1>Testing Child Process Timeout</h1>"
echo "<p>This process will remain active for 60 seconds to test timeout behavior.</p>"

# Simulate a long-running child process (the one to be tested for timeout)
echo "<p>The child process is running...</p>"

# Start a background process that sleeps for 60 seconds
sleep 60 &

# Wait for the background process to finish
wait

# End HTML content after the background process has finished
echo "<p>Done! The process has completed after 60 seconds.</p>"
echo "</body></html>"
