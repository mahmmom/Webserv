#!/bin/bash

# Set the Content-Type header for plain text
echo "Content-type: text/html"
echo ""

# Output the HTML content using a here document
cat <<EOF
<html>
<head>
    <title>Dynamic Page</title>
</head>
<body>
    <h1 id="message">Hello Boo</h1>

    <script>
    let count = 0;
    function updateMessage() {
        if (count < 12) {
            document.getElementById('message').innerText = 'Hello Boo ' + (count + 1);
            count++;
        } else {
            document.getElementById('message').innerText = 'Done!';
        }
    }
    setInterval(updateMessage, 5000);
    </script>

</body>
</html>
EOF