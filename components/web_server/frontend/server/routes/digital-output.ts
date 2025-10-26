import { Router } from "express";

const digitalInputRouter = Router();

const outputs = [false, false, false, false];

digitalInputRouter.post("/digital-output/:id", (request, response) => {
  const { params, body } = request;
  const { state } = body;

  const id = Number(params.id);

  outputs[id] = state;

  response.json({ state: outputs[id] });
});

digitalInputRouter.get("/digital-output/:id", (request, response) => {
  const { params } = request;

  const id = Number(params.id);

  response.json({ state: outputs[id] });
});

export { digitalInputRouter };