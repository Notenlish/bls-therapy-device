import { error } from '@sveltejs/kit';

export const POST = async ({request}) => {
    const data = await request.json()
    const msg = data.msg;


    
    return new Response(JSON.stringify({ "connected": "true" }), { "status": 200 });
};