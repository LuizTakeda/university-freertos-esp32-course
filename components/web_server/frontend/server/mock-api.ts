import express from "express"
import cors from "cors"
import { digitalOutputRouter } from "./api/digital-output";
import { digitalInputRouter } from "./api/digital-input";
import { eventsRouter } from "./api/events";

const app = express();

app.use(cors());
app.use(express.json());

app.get("/api", (request, response) => {
  response.send("mock api is running");
});

app.use("/api", digitalOutputRouter);
app.use("/api", digitalInputRouter);
app.use("/api", eventsRouter);

app.listen(4000, () => {
  console.log("running at http://localhost:4000");
});