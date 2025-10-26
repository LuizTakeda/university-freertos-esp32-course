import { Router } from "express";

const digitalInputRouter = Router();

const outputs = [false, false, false, false];

digitalInputRouter.post("/api/digital-output/:id", (request, response) => {
  const { params, body } = request;
  const { state } = body;

  const id = Number(params.id);

  outputs[id] = state;

  response.json({ state: outputs[id] });
});

digitalInputRouter.get("/api/digital-output/:id", (request, response) => {
  const { params } = request;

  const id = Number(params.id);

  response.json({ state: outputs[id] });
});

export { digitalInputRouter };