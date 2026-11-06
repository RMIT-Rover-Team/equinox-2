import './styles/global.scss';

import { useState } from 'react';

import type { InfoBox } from './components/banner/Banner';
import Layout from './components/layout/Layout';
import type { Page } from './components/navbar/Navbar';
import Drive from './views/drive/Drive';
import Arm from './views/arm/Arm';
import Science from './views/science/Science';
import Excavator from './views/excavator/Excavator';

// Placeholders until these come from the backend
const infoBoxes: InfoBox[] = [
	{ label: 'Info 1', value: '--' },
	{ label: 'Info 2', value: '--' },
	{ label: 'Info 3', value: '--' },
];

function App() {
	const [page, setPage] = useState<Page>('Drive');
	const stationNumber = page === 'Drive' ? 1 : 2;

	return (
		<Layout
			page={page}
			onPageChange={setPage}
			stationNumber={stationNumber}
			infoBoxes={infoBoxes}
		>
			{page === 'Drive' && <Drive />}
			{page === 'Arm' && <Arm />}
			{page === 'Science' && <Science />}
			{page === 'Excavator' && <Excavator />}
		</Layout>
	);
}

export default App;
