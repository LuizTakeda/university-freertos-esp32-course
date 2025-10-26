import express from "express"
import cors from "cors"
import { digitalInputRouter } from "./routes/digital-output";

const app = express();

app.use(cors());
app.use(express.json());

app.get("/api", (request, response) => {
  response.send("mock api is running");
});

app.use("/api", digitalInputRouter);

app.listen(4000, () => {
  console.log("running at http://localhost:4000");
});