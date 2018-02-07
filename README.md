"Pure C" group's bot

A friendly community of hacker programmers. We will be happy to help you or talk about the exciting world of code.

Group language: RU\EN

Link: https://web.telegram.org/#/im?p=@Pure_C

Ver 0.1.0         
makfloy created an unstable test version

##For install this bot. 
<i>$ openssl req -newkey rsa:2048 -sha256 -nodes -keyout private.key -x509 -days 365 -out public.pem </i>
Inter your IP server.

<i>$ cat private.key public.pem > cert.pem</i>

<i>$ curl  -F"url=https://$IP:8443" -F"certificate=@public.pem" -F"max_connections=40" https://api.telegram.org/bot$TOKEN/setWebhook </i>
$IP is your IP server

Find and replace your TOKEN in the pure_c.c

<i>$ gcc pure_c.c -o bot -lcrypto -lssl</i>
