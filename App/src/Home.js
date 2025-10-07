import React from 'react';

function Home({ onNavigateTuner, onNavigateLearning }) {
  return (
    <div>
      <h1>Welcome to the Stringless Violin Companion App</h1>
      <button onClick={onNavigateTuner}>Go to Tuner</button>
      <button onClick={onNavigateLearning}>Go to Learning Tools</button>
    </div>
  );
}

export default Home;
