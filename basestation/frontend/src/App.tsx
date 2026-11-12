import './styles/global.scss';

import { useState, createContext } from 'react';
import type { ReactNode } from 'react';
import CameraRoom from './components/cameras/CameraRoom';

import type { InfoBox } from './components/banner/Banner';
import Layout from './components/layout/Layout';
import type { Page } from './types/Pages';
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

const views: Record<Page, ReactNode> = {
	Drive: <Drive />,
	Arm: <Arm />,
	Science: <Science />,
	Excavator: <Excavator />,
};

type Theme = 'Dark' | 'Light';

function App() {
	const [page, setPage] = useState<Page>('Drive');
	const themeContext = createContext<Theme>('Dark');
	return (
		<CameraRoom>
			<Layout
				page={page}
				onPageChange={setPage}
				infoBoxes={infoBoxes}
			>
				{views[page]}
			</Layout>
		</CameraRoom>
	);
}

export default App;
