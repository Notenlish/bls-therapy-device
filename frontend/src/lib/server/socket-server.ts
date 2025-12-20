// socket-server.ts

import * as net from "net"
import { writable } from 'svelte/store'; // Optional, for real-time status

const HOST = '0.0.0.0'
const PORT = 8290

// the svelte server.ts file will push new messages that will be sent to connected devices.
// message is just a simple string.
export const messageQueue: string[] = [];

const clients = new Set<net.Socket>();

const server = net.createServer((sock) => {
    console.log("Client connected:", sock.remoteAddress);

    clients.add(sock);

    sock.on("data", (data) => {
        console.log("Received:", data.toString());
    });

    sock.on("close", () => {
        clients.delete(sock);
        console.log("Client disconnected");
    });

    sock.on("error", () => {
        clients.delete(sock);
    });
});

server.listen(PORT, HOST, () => {
    console.log(`TCP server listening on ${HOST}:${PORT}`);
});

// Message Push Logic
setInterval(() => {
    if (clients.size === 0) return;

    let msg;
    if (messageQueue.length === 0) {
        // if no message exists, check back in 200ms.
        setTimeout(() => {
            // msg still doesnt exist, send a HRBT(heartbeat)
            if (messageQueue.length === 0) {
                msg = "HRBT"
            } else {
                return;
            }
        }, 200)
    } else {
        msg = messageQueue.shift();
        console.log(`I WILL BE SENDING THIS MESSAGE "${msg}"`)
    }
    if (!msg) return;

    for (const client of clients) {
        if (!client.destroyed) {
            client.write(msg + "\n");
        }
    }
}, 10);

// Shutdown
process.on("SIGINT", () => {
    console.log("Shutting down...");
    for (const client of clients) {
        client.destroy();
    }
    server.close(() => process.exit(0));
});