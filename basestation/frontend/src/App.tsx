import './App.css'
import { useState } from 'react'

function App() {
  type Page = 'Cameras' | 'Drive' | 'Arm' | 'Science' | 'Excavator'

  const [page, setPage] = useState<Page>('Cameras');
  return (
    <>
      <nav>
        <button onClick={() => setPage('Cameras')}>Cameras</button>
        <button onClick={() => setPage('Drive')}>Cameras</button>
        <button onClick={() => setPage('Arm')}>Cameras</button>
        <button onClick={() => setPage('Science')}>Cameras</button>
        <button onClick={() => setPage('Excavator')}>Cameras</button>
      </nav>


      {/*{page == 'Cameras' && <Cameras />}
      {page == 'Drive' && <Drive />}
      {page == 'Arm' && <Arm />}
      {page == 'Science' && <Science />}
      {page == 'Excavator' && <Excavator />}*/}

    </>
  )
}

export default App
