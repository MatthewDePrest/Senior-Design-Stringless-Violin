import React, { useState } from 'react';

// Import or define your components below
import Home from './Home';
import Tuner from './Tuner';
import LearningTools from './LearningTools';

function App() {
  // Manage which screen is currently shown
  const [currentPage, setCurrentPage] = useState('home');

  // Function to render the current page based on state
  function renderPage() {
    switch (currentPage) {
      case 'tuner':
        return <Tuner onBack={() => setCurrentPage('home')} />;
      case 'learning':
        return <LearningTools onBack={() => setCurrentPage('home')} />;
      case 'home':
      default:
        return <Home 
          onNavigateTuner={() => setCurrentPage('tuner')} 
          onNavigateLearning={() => setCurrentPage('learning')}
        />;
    }
  }

  return (
    <div className="App">
      {renderPage()}
    </div>
  );
}

export default App;
