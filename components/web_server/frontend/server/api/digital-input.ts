import { Router } from "express";

const digitalInputRouter = Router();

const inputs = [false, true, false, true];

digitalInputRouter.post("/digital-input", (request, response) => {
  const { query, body } = request;
  const { state } = body;

  const id = Number(query.id);

  inputs[id] = state;

  response.json({ state: inputs[id] });
});

digitalInputRouter.get("/digital-input", (request, response) => {
  const { query } = request;

  const id = Number(query.id);

  response.json({ state: inputs[id] });
});

export { digitalInputRouter };