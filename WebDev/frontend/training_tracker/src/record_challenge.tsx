import React, { useState, useEffect, useRef } from 'react';
import { useLocation } from 'react-router-dom';

interface Segment {
  id: number;
  name: string;
  time?: number;
}

interface Challenge {
  challengeName: string;
  segments: Segment[];
}

const Record_Challenge: React.FC = () => {
  const location = useLocation();
  const challengeData = location.state as { challenge: Challenge } | null;

  const [isRunning, setIsRunning] = useState(false);
  const [isPaused, setIsPaused] = useState(false);
  const [time, setTime] = useState(0);
  const [segmentTimes, setSegmentTimes] = useState<Segment[]>(challengeData?.challenge.segments || []);
  const timerRef = useRef<number | null>(null);

  useEffect(() => {
    if (isRunning && !isPaused) {
      timerRef.current = window.setInterval(() => {
        setTime(prevTime => prevTime + 1);
      }, 1000);
    } else if (timerRef.current) {
      clearInterval(timerRef.current);
      timerRef.current = null;
    }

    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current);
      }
    };
  }, [isRunning, isPaused]);

  const handleStart = () => {
    setIsRunning(true);
  };

  const handleNextSegment = () => {
    const nextSegmentIndex = segmentTimes.findIndex(segment => segment.time === undefined);
    if (nextSegmentIndex !== -1) {
      const newSegmentTimes = [...segmentTimes];
      newSegmentTimes[nextSegmentIndex].time = time;
      setSegmentTimes(newSegmentTimes);
    }
  };

  const handleEnd = () => {
    setIsRunning(false);
  };

  const handlePauseResume = () => {
    setIsPaused(prevIsPaused => !prevIsPaused);
  };

  return (
    <div>
      <h1>{challengeData?.challenge.challengeName}</h1>
      <div>
        <h2>Overall Timer: {time}s</h2>
      </div>
      <ul>
        {segmentTimes.map((segment, index) => (
          <li key={segment.id}>
            {segment.name}: {segment.time !== undefined ? `${segment.time}s` : 'Not completed'}
          </li>
        ))}
      </ul>
      {!isRunning && <button onClick={handleStart}>Start</button>}
      {isRunning && !isPaused && (
        <>
          <button onClick={handleNextSegment}>Next Segment</button>
          <button onClick={handleEnd}>End</button>
          <button onClick={handlePauseResume}>Pause</button>
        </>
      )}
      {isRunning && isPaused && (
        <button onClick={handlePauseResume}>Resume</button>
      )}
    </div>
  );
};

export default Record_Challenge;
