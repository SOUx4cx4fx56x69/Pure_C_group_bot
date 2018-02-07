# "Pure C" group's bot

A friendly community of hacker programmers. We will be happy to help you or talk about the exciting world of code.

Group language: RU\EN

Link: https://web.telegram.org/#/im?p=@Pure_C

Ver 0.1.0         
makfloy created an unstable test version

## For install this bot.

>$ openssl req -newkey rsa:2048 -sha256 -nodes -keyout private.key -x509 -days 365 -out public.pem

Inter your IP server.

>$ cat private.key public.pem > cert.pem

>$ curl  -F"url=https://$IP:8443" -F"certificate=@public.pem" -F"max_connections=40" https://api.telegram.org/bot$TOKEN/setWebhook

$IP is your IP server

Find and replace your TOKEN in the pure_c.c

>const char BOT_TOKEN[]   = "";

>$ gcc pure_c.c -o bot -lcrypto -lssl

