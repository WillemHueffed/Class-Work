import { useState } from 'react'
import reactLogo from './assets/react.svg'
import viteLogo from '/vite.svg'
import './App.css'
import StopSearch from './StopSearch'

function App() {
  const [count, setCount] = useState(0)

  const onClick = async () => {
    setCount((count) => count + 1)
    
  }

  return (
    <>
      <div className="overlay">
        <div><span>Test</span></div>
        <div><span>Test 2</span></div>
      </div>
      <StopSearch />
    </>
  )
}

export default App
