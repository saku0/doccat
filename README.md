# doccat
This program allow you to concatenate .doc files using LibreOffice.

If you are not upto compiling the C++ program, I have a prebuilt ```doccat```
executable for Ubuntu in the sandbox folder.

First run LibreOffice in headless mode:
```sh
  $ soffice.bin --headless --accept=socket,host=127.0.0.1,port=8100,tcpNoDelay=1;urp;
```
Now, catenate the files:
```sh
  $ ./doccat -o out.doc in1.doc in2.doc in3.doc
```

Note: The ./doccat executable uses ./_doccat which is a copy of the unoapploader
application (from the SDK). This is also included in the sandbox.
