import { Router } from "express";

const eventsRouter = Router();

function randomInt(min: number, max: number): number {
  return Math.floor(Math.random() * (max - min)) + min;
}

function generateDigitalInputEvent() {
  return {
    num: randomInt(0, 4),
    value: randomInt(0, 2)
  }
}

eventsRouter.get("/events", (request, response) => {
  response.setHeader('Content-Type', 'text/event-stream; charset=utf-8');
  response.setHeader('Cache-Control', 'no-cache, no-transform');
  response.setHeader('Connection', 'keep-alive');

  response.flushHeaders();

  setInterval(() => {
    response.write(
      `event: digital-input\n` +
      `data: ${JSON.stringify(generateDigitalInputEvent())}\n\n`
    );
  }, 1000);

  const AMPLITUDE = 10;
  const OFFSET = 50;
  const FREQUENCY_STEP = 100;
  let waveAngle = 0;

  setInterval(() => {
    let sineValue = OFFSET + (AMPLITUDE * Math.sin(waveAngle));

    let chartValue = Math.round(sineValue);

    waveAngle += FREQUENCY_STEP;

    response.write(
      `event: analog-input\n` +
      `data: ${JSON.stringify({
        num: 0,
        value: chartValue
      })}\n\n`
    );
  }, 100);


  response.on('close', () => {
    console.log(`Client disconnected`);
  });
});

export { eventsRouter }