import { Router } from "express";

const digitalInputRouter = Router();

const inputs = [false, true, false, true];

digitalInputRouter.post("/digital-input/:id", (request, response) => {
  const { params, body } = request;
  const { state } = body;

  const id = Number(params.id);

  inputs[id] = state;

  response.json({ state: inputs[id] });
});

digitalInputRouter.get("/digital-input/:id", (request, response) => {
  const { params } = request;

  const id = Number(params.id);

  response.json({ state: inputs[id] });
});

export { digitalInputRouter };