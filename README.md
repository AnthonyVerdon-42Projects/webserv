# Webserv 

## Why does a URL start with HTTP ?

This project is part of the fifth circle of projects of the 42 common core, and it's the third one in group. The goal is to create a web server. For that, I was with my friend Théo Guerin (login 42 : tguerin, Github account : https://github.com/TheoGuerin64) and Unai Layus (login 42 : ulayus, Github account : https://github.com/0x35c). You will find the subject of the project into the repository. My grade : 125/100

## How to use it

1. Clone it `git clone git@github.com:AnthonyVerdon-42Projects/webserv.git`.
2. Compile it `make`.
3. Start the server by executing the program. You can give it a parameter, which should be a configuration file (like NGINX one). You can find an example in `conf/example.conf`, or use the base one  `conf/base.conf`.
4. After you started the server you can open a browser (in preference Google Chrome). You also need to have files like HTML, CSS or Javascript one. The server will handle GET, POST and DELETE requests you make (if they are enable in the config file). It also handle directory listing and CGI.

## Find a bug ?

If you find an undefined behaviour (crash, leaks, ...), please submit an issue or contact me
