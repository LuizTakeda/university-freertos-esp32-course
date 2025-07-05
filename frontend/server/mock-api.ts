import express from "express"
import cors from "cors"

const app = express();

app.use(cors());
app.use(express.json());

app.get("/api", (request, response) => {
  response.send("mock api is running");
});

app.listen(4000, () => {
  console.log("running at http://localhost:4000");
});