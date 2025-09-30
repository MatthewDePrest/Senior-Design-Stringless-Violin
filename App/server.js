// server.js
const express = require('express');
const fs = require('fs');
const cors = require('cors');

const app = express();
const PORT = 5000;

app.use(cors());
app.use(express.json());

app.post('/save-frequencies', (req, res) => {
  const frequencies = req.body;

  const fileContent =
    `${frequencies.First}.0\n` +
    `${frequencies.Second}.0\n` +
    `${frequencies.Third}.0\n` +
    `${frequencies.Fourth}.0\n`;

  fs.writeFile('violin_frequencies.txt', fileContent, (err) => {
    if (err) {
      console.error('Error writing file:', err);
      return res.status(500).send('Error saving file');
    }
    res.send('File saved successfully');
  });
});

app.listen(PORT, () => {
  console.log(`Server listening on port ${PORT}`);
});
