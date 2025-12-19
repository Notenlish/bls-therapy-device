import { error } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

export const GET: RequestHandler = () => {
    return new Response(JSON.stringify({ "connected": "true" }), { "status": 200 });
};