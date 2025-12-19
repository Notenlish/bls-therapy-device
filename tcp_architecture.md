# tcp architecture

4 letters codes.

OPEN --> OPEN's connection(used to check if connected)
HRBT --> Heartbeat
SETP [float between 0 and 1] --> set power of motors
GETB --> get battery level --> responds back with BATL [float between 0 and 1]
CLOS --> CLOSes connection
STRT --> STaRTs the motors
STOP --> STOPs the motors


the backend will basically act as a proxy for the frontend(client)

client sends http requests --> the json body converted into meaningful socket requests --> backend sends them --> gets the response --> replies back to the frontend.
