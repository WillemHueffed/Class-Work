import React, { useState } from 'react';
interface Segment {
  id: number;
  name: string;
}

const Record_Challenge: React.FC = () => {
  const [challengeName, setChallengeName] = useState('');
  const [segments, setSegments] = useState<Segment[]>([]);
  const [nextSegmentId, setNextSegmentId] = useState(1);

  const handleChallengeNameChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setChallengeName(e.target.value);
  };

  const handleSegmentNameChange = (id: number, newName: string) => {
    setSegments(prevSegments =>
      prevSegments.map(segment =>
        segment.id === id ? { ...segment, name: newName } : segment
      )
    );
  };

  const addSegment = () => {
    setSegments([...segments, { id: nextSegmentId, name: 'New Segment' }]);
    setNextSegmentId(nextSegmentId + 1);
  };

  const removeSegment = (id: number) => {
    setSegments(prevSegments => prevSegments.filter(segment => segment.id !== id));
  };

  const saveChallenge = () => {
    if (challengeName.trim() && segments.length > 0) {
      const newChallenge = { challengeName, segments };
      const savedChallenges = JSON.parse(localStorage.getItem('challenges') || '[]');
      savedChallenges.push(newChallenge);
      localStorage.setItem('challenges', JSON.stringify(savedChallenges));
      
      console.log('Challenge saved:', newChallenge);
      // Reset the form or navigate to another page
      setChallengeName('');
      setSegments([]);
    } else {
      alert('Please enter a challenge name and at least one segment.');
    }
  };

  const cancel = () => {
    // Reset the form or navigate to another page
    setChallengeName('');
    setSegments([]);
  };

  return (
    <div>
      <h1>Create a New Challenge</h1>

      <label>
        Challenge Name:
        <input type="text" value={challengeName} onChange={handleChallengeNameChange} />
      </label>

      <h2>Segments</h2>
      {segments.map(segment => (
        <div key={segment.id}>
          <input
            type="text"
            value={segment.name}
            onChange={e => handleSegmentNameChange(segment.id, e.target.value)}
          />
          <button onClick={() => removeSegment(segment.id)}>Remove</button>
        </div>
      ))}

      <button onClick={addSegment}>Add Segment</button>
      <br />
      <button onClick={cancel}>Cancel</button>
      <button onClick={saveChallenge}>Save Challenge</button>
    </div>
  );
};

export default Record_Challenge 
