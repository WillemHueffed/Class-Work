import React from 'react'
import ReactDOM from 'react-dom/client'
import List_Challenges from './list_challenges.tsx';
import Create_Challenge from './create_challenge.tsx';
import Record_Challenge from './record_challenge.tsx';
import './index.css'
import { createBrowserRouter, RouterProvider } from 'react-router-dom';

const router = createBrowserRouter([
{
  path: "/",
  element: <List_Challenges/>,
},
{
  path: "/create_challenge",
  element: <Create_Challenge/>,
},
{
  path: "/record_challenge",
  element: <Record_Challenge/>,
}
]);

ReactDOM.createRoot(document.getElementById('root')!).render(
  <React.StrictMode>
    <RouterProvider router = {router} />
  </React.StrictMode>,
)
