import express, { Response } from "express"
import cors from "cors"

const clients: Set<Response> = new Set();

const publishEvent = (event: string, data: object) => {
  console.log("pub");
  clients.forEach((client) => {
    client.write(JSON.stringify({ event, data }));
  });
}

const output: Array<boolean> = [false, false, false];

const app = express();

app.use(cors());
app.use(express.json());

app.get("/api", (request, response) => {
  response.send("mock api is running");
});

app.get("/api/events", (request, response) => {
  // Set SSE headers
  response.set({
    'Content-Type': 'text/event-stream',
    'Cache-Control': 'no-cache',
    'Connection': 'keep-alive'
  });

  response.flushHeaders();

  clients.add(response);
  console.log("New client");

  response.on('close', () => {
    clients.delete(response);
    console.log("Client close");
  });
});

app.get("/api/output", (request, response) => {
  response.json(output);
});

app.post("/api/output/:id", (request, response) => {
  const id = Number(request.params.id);
  const { value } = request.body;

  if (isNaN(id) || id < 0 || id >= 3) {
    response.status(500).send("invalid id");
    return;
  }

  if (typeof value !== "boolean") {
    response.status(500).send("invalid value");
    return;
  }

  publishEvent("output", { id, value });

  output[id] = value;

  response.send("success");
});

app.listen(4000, () => {
  console.log("running at http://localhost:4000");
});