import { Router } from "express";

const digitalOutputRouter = Router();

const outputs = [false, false, false, false];

digitalOutputRouter.post("/digital-output", (request, response) => {
  const { body, query } = request;
  const { state } = body;

  const id = Number(query.id);

  outputs[id] = state;

  response.json({ state: outputs[id] });
});

digitalOutputRouter.get("/digital-output", (request, response) => {
  const { query } = request;

  const id = Number(query.id);

  response.json({ state: outputs[id] });
});

export { digitalOutputRouter };