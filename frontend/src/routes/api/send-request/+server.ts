// server.ts (handle requests here)

import { error } from '@sveltejs/kit';
import { type RequestHandler } from '@sveltejs/kit';
import { messageQueue } from '$lib/server/socket-server';

export type CommandType = "OPEN" | "HRBT" | "SETP" | "GETB" | "CLOS" | "STRT" | "STOP" | "NONE"

export const POST: RequestHandler = async ({ request }) => {
    const data: { command_name: string, payload: string } = await request.json()
    if (data.command_name == "NONE") {
        return new Response(JSON.stringify({ "message": "Starting the server." }))
    }
    const message = `${data.command_name} ${data.payload}\n`;

    messageQueue.push(message)

    return new Response(JSON.stringify({ "message": `Successfully added message "${message}" to be sent to the esp32.` }), { "status": 200 });
};