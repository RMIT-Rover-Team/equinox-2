import type { ReactNode } from 'react';

import Banner from '../banner/Banner';
import type { InfoBox } from '../banner/Banner';
import Cameras from '../cameras/Cameras';
import Navbar from '../navbar/Navbar';
import type { Page } from '../navbar/Navbar';
import styles from './Layout.module.scss';

type LayoutProps = {
	page: Page;
	onPageChange: (page: Page) => void;
	stationNumber: number;
	infoBoxes: InfoBox[];
	children: ReactNode;
};

function Layout({ page, onPageChange, stationNumber, infoBoxes, children }: LayoutProps) {
	return (
		<div className={styles.layout}>
			<Banner
				pageName={page}
				stationNumber={stationNumber}
				infoBoxes={infoBoxes}
			/>
			<Navbar
				page={page}
				onPageChange={onPageChange}
			/>
			<div className={styles.body}>
				<section className={styles.panel}>
					<Cameras />
				</section>
				<main className={styles.panel}>{children}</main>
			</div>
		</div>
	);
}

export default Layout;
