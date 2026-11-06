import { useState } from 'react'

import Cameras from '../../views/cameras/Cameras'
import Drive from '../../views/drive/Drive'
import Arm from '../../views/arm/Arm'
import Science from '../../views/science/Science'
import Excavator from '../../views/excavator/Excavator'

function Navbar() {
  type Page = 'Cameras' | 'Drive' | 'Arm' | 'Science' | 'Excavator'

  const [page, setPage] = useState<Page>('Cameras');
  return (
    <>
      <nav>
        <button onClick={() => setPage('Cameras')}>Cameras</button>
        <button onClick={() => setPage('Drive')}>Drive</button>
        <button onClick={() => setPage('Arm')}>Arm</button>
        <button onClick={() => setPage('Science')}>Science</button>
        <button onClick={() => setPage('Excavator')}>Excavator</button>
      </nav>


      { page == 'Cameras' && <Cameras /> }
      { page == 'Drive' && <Drive /> }
      { page == 'Arm' && <Arm /> }
      { page == 'Science' && <Science /> }
      { page == 'Excavator' && <Excavator />}
    </>
  )
}

export default Navbar
