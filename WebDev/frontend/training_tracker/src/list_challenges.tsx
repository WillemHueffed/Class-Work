import React, { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';

interface Segment {
  id: number;
  name: string;
}

interface Challenge {
  challengeName: string;
  segments: Segment[];
}

const List_Challenge: React.FC = () => {
  const [challenges, setChallenges] = useState<Challenge[]>([]);
  const navigate = useNavigate();

  useEffect(() => {
    const savedChallenges = JSON.parse(localStorage.getItem('challenges') || '[]');
    setChallenges(savedChallenges);
  }, []);

  const goToChallengeRecordingScreen = (challenge: Challenge) => {
    navigate('/record_challenge', { state: { challenge } });
  };

  const goToChallengeCreationScreen = () => {
    navigate('/create_challenge');
  };

  return (
    <div>
      <h1>Your Challenges</h1>
      <ul>
        {challenges.map((challenge, index) => (
          <li key={index}>
            <span>{challenge.challengeName}</span>
            <button onClick={() => goToChallengeRecordingScreen(challenge)}>
              Go to Challenge
            </button>
          </li>
        ))}
      </ul>
      <button onClick={goToChallengeCreationScreen}>Create New Challenge</button>
    </div>
  );
};

export default List_Challenge;
