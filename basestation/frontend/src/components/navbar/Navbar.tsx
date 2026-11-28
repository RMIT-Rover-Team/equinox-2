import styles from './Navbar.module.scss';
import { pages, type Page } from '../../types/Pages';


type NavbarProps = {
	page: Page;
	onPageChange: (page: Page) => void;
};

function Navbar({ page, onPageChange }: NavbarProps) {
	return (
		<nav className={styles.navbar}>
			{pages.map((name) => (
				<button
					key={name}
					className={styles.button}
					aria-current={name === page ? 'page' : undefined}
					onClick={() => onPageChange(name)}
				>
					{name}
				</button>
			))}
		</nav>
	);
}

export default Navbar;
