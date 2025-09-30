import React from 'react';

function LearningTools({ onBack }) {
  return (
    <div>
      <button onClick={onBack}>Back</button>
      <h2>Learning Tools</h2>
      {/* Placeholder: Insert sheet music rendering component here */}
      <p>Sheet music will be displayed here.</p>
      {/* Add interactive practice tools, note input, feedback */}
    </div>
  );
}

export default LearningTools;
