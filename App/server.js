// server.js
const express = require('express');
const fs = require('fs');
const cors = require('cors');
const path = require('path');

const app = express();
const PORT = 5000;

app.use(cors());
app.use(express.json());

const LIVE_DIR = path.join(__dirname, 'live');
if (!fs.existsSync(LIVE_DIR)) fs.mkdirSync(LIVE_DIR);

// Serve /live/notes.txt as a static file
app.get('/live/notes.txt', (req, res) => {
  const filePath = path.join(LIVE_DIR, 'notes.txt');
  res.sendFile(filePath);
});

// Example: write to that file from your other process
app.post('/save-notes', (req, res) => {
  const notes = req.body.notes;
  fs.writeFile(path.join(LIVE_DIR, 'notes.txt'), notes.join(' '), (err) => {
    if (err) return res.status(500).send('Failed to write notes');
    res.send('Notes saved');
  });
});

// Clear notes.txt
app.delete('/live/notes.txt', (req, res) => {
  const filePath = path.join(LIVE_DIR, 'notes.txt');
  fs.writeFile(filePath, '', (err) => {
    if (err) {
      console.error('Error clearing notes.txt:', err);
      return res.status(500).send('Failed to clear notes.txt');
    }
    res.send('notes.txt cleared successfully');
  });
});

// Save violin frequencies to a .txt file
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

app.listen(PORT, () => console.log(`Server running on port ${PORT}`));
