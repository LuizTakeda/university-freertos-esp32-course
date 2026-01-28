import { Router } from "express";

const eventsRouter = Router();

const randomInt = (min: number, max: number) => Math.floor(Math.random() * (max - min)) + min;

eventsRouter.get("/events", (request, response) => {
  response.setHeader('Content-Type', 'text/event-stream; charset=utf-8');
  response.setHeader('Cache-Control', 'no-cache, no-transform');
  response.setHeader('Connection', 'keep-alive');
  response.flushHeaders();

  const sendEvent = (event: string, data: object) => {
    response.write(`event: ${event}\n`);
    response.write(`data: ${JSON.stringify(data)}\n\n`);
  };

  const digitalInterval = setInterval(() => {
    sendEvent("digital-input", {
      num: randomInt(0, 4),
      value: randomInt(0, 2)
    });
  }, 1000);

  let waveAngle = 0;
  const analogInterval = setInterval(() => {
    waveAngle += 0.1;

    for (let i = 0; i < 4; i++) {
      let value = 0;

      switch (i) {
        case 0:
          value = Math.round(2000 + (200 * Math.sin(-waveAngle)));
          break;

        case 1:
          value = Math.round(2000 + (200 * Math.sin(waveAngle)));
          break;

        case 2:
          value = Math.round(25 + (2 * Math.sin(waveAngle)));
          break;

        case 3:
          value = Math.round(60 + (2 * Math.sin(waveAngle)));
          break;

        default:
      }

      sendEvent("analog-input", { num: i, value });
    }
  }, 200);

  request.on('close', () => {
    clearInterval(digitalInterval);
    clearInterval(analogInterval);
    console.log(`Client disconnected - Intervals cleared`);
    response.end();
  });
});

export { eventsRouter };