// for esp32c3

import * as net from "net"
import { writable } from 'svelte/store'; // Optional, for real-time status

const msgQueue = [];

let esp32Socket: net.Socket | null = null;

const server = net.createServer((sock) => {
    sock.write("Echo server\r\n");
    console.log('client connected');
    sock.on('end', () => {
        console.log('client disconnected');
    });
    sock.write('hello\r\n');
    sock.pipe(sock);
})

server.on("error", (err) => {
    throw err;
})

server.listen(8290, '0.0.0.0');