import React from 'react';
import './App.css';

function Home({ onNavigateTuner, onNavigateLearning, onNavigateComposer }) {
  return (
    <div className="App-header">
      <h1>Welcome to the Stringless Violin Companion App</h1>
      
      <div className="home-buttons">
        <button className="tuner-btn" onClick={onNavigateTuner}>
          Tuner
        </button>
        <button className="learning-btn" onClick={onNavigateLearning}>
          Learning Tool
        </button>
        <button className="worksheet-btn" onClick={onNavigateComposer}>
          Music Composer
        </button>
      </div>
    </div>
  );
}

export default Home;
