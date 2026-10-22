// Apex Drive - C++ OOP Car Rental Client
document.addEventListener('DOMContentLoaded', () => {
    // State
    let fleet = [];
    let rentals = [];
    let activeFilter = 'all';
    let selectedCarForBooking = null;

    // DOM Elements
    const fleetContainer = document.getElementById('fleet-container');
    const rentalsTbody = document.getElementById('rentals-tbody');
    const historyTbody = document.getElementById('history-tbody');
    const filterButtons = document.querySelectorAll('.filter-btn');
    
    // Stats
    const statTotal = document.getElementById('stat-total');
    const statAvailable = document.getElementById('stat-available');
    const statRented = document.getElementById('stat-rented');
    const statRevenue = document.getElementById('stat-revenue');

    // Booking Modal
    const bookingModal = document.getElementById('booking-modal');
    const bookForm = document.getElementById('book-form');
    const bookCarIdInput = document.getElementById('book-car-id');
    const bookCustomerInput = document.getElementById('book-customer');
    const bookDaysInput = document.getElementById('book-days');
    const modalCarBadge = document.getElementById('modal-car-badge');
    const modalCarName = document.getElementById('modal-car-name');
    const modalCarRate = document.getElementById('modal-car-rate');
    const calcDays = document.getElementById('calc-days');
    const calcRate = document.getElementById('calc-rate');
    const calcBase = document.getElementById('calc-base');
    const calcTotal = document.getElementById('calc-total');
    const suvSurchargeRow = document.getElementById('suv-surcharge-row');
    const modalCloseBtn = document.getElementById('modal-close-btn');
    const modalCancelBtn = document.getElementById('modal-cancel-btn');

    // Add Car Form
    const addCarForm = document.getElementById('add-car-form');
    const refreshRentalsBtn = document.getElementById('refresh-rentals-btn');

    // ----------------- Toast Notifications -----------------
    function showToast(message, type = 'success') {
        const container = document.getElementById('toast-container');
        const toast = document.createElement('div');
        toast.className = `toast toast-${type}`;
        toast.innerHTML = `<span>${type === 'success' ? '✓' : '⚠️'}</span><span>${message}</span>`;
        container.appendChild(toast);
        setTimeout(() => {
            toast.style.opacity = '0';
            setTimeout(() => toast.remove(), 200);
        }, 3500);
    }

    // ----------------- API Calls -----------------
    async function fetchFleet() {
        try {
            const res = await fetch('/api/cars');
            if (!res.ok) throw new Error('Failed to load fleet');
            fleet = await res.json();
            renderFleet();
            updateStats();
        } catch (err) {
            console.error(err);
            showToast('Error connecting to C++ backend: ' + err.message, 'error');
            fleetContainer.innerHTML = '<div class="text-center text-muted" style="grid-column: 1/-1;">Could not load vehicle inventory from C++ server.</div>';
        }
    }

    async function fetchRentals() {
        try {
            const res = await fetch('/api/rentals');
            if (!res.ok) throw new Error('Failed to load rentals');
            rentals = await res.json();
            renderRentals();
            renderHistory();
            updateStats();
        } catch (err) {
            console.error(err);
            showToast('Error loading rentals: ' + err.message, 'error');
        }
    }

    // ----------------- Render Functions -----------------
    function renderFleet() {
        let filtered = fleet;
        if (activeFilter === 'Sedan') {
            filtered = fleet.filter(c => c.type === 'Sedan');
        } else if (activeFilter === 'SUV') {
            filtered = fleet.filter(c => c.type === 'SUV');
        } else if (activeFilter === 'available') {
            filtered = fleet.filter(c => c.available);
        }

        if (filtered.length === 0) {
            fleetContainer.innerHTML = '<div class="text-center text-muted" style="grid-column: 1/-1; padding: 2rem;">No vehicles found matching current filter.</div>';
            return;
        }

        fleetContainer.innerHTML = filtered.map(car => {
            const isSUV = car.type === 'SUV';
            const badgeClass = isSUV ? 'badge-suv' : 'badge-sedan';
            const statusClass = car.available ? 'status-available' : 'status-rented';
            const statusText = car.available ? '● Available' : '● Currently Rented';
            const bookBtnDisabled = !car.available ? 'btn-disabled disabled' : '';

            return `
                <div class="car-card ${car.available ? '' : 'rented'}">
                    <div>
                        <div class="card-top">
                            <span class="car-id-tag">ID: #${car.id}</span>
                            <span class="badge ${badgeClass}">${car.type}</span>
                        </div>
                        <h3 class="car-name-title">${car.name}</h3>
                        <p class="car-type-subtitle">${isSUV ? 'SUV • Includes Rs. 300 Surcharge' : 'Sedan • Standard Mileage'}</p>
                    </div>

                    <div>
                        <div class="card-pricing">
                            <div>
                                <span class="rate-amount">₹${car.rate.toLocaleString('en-IN')}</span>
                                <span class="rate-period"> / day</span>
                            </div>
                            <span class="status-badge ${statusClass}">${statusText}</span>
                        </div>

                        <div style="margin-top: 1rem;">
                            <button class="btn btn-primary ${bookBtnDisabled}" 
                                    style="width: 100%;" 
                                    ${car.available ? `onclick="openBookingModal(${car.id})"` : 'disabled'}>
                                ${car.available ? 'Book Vehicle' : 'Unavailable'}
                            </button>
                        </div>
                    </div>
                </div>
            `;
        }).join('');
    }

    function renderRentals() {
        const activeRentals = rentals.filter(r => r.active);
        if (activeRentals.length === 0) {
            rentalsTbody.innerHTML = '<tr><td colspan="7" class="text-center text-muted" style="padding: 1.5rem;">No active rentals at the moment. All cars are at the hub.</td></tr>';
            return;
        }

        rentalsTbody.innerHTML = activeRentals.map(r => `
            <tr>
                <td class="font-mono">#${r.rentalId}</td>
                <td><strong>${escapeHtml(r.customer)}</strong></td>
                <td>${escapeHtml(r.carName)} <span class="car-id-tag">#${r.carId}</span></td>
                <td>${r.days} days</td>
                <td class="font-mono text-cyan">₹${r.amount.toLocaleString('en-IN', { minimumFractionDigits: 2 })}</td>
                <td><span class="status-badge status-rented">Active</span></td>
                <td>
                    <button class="btn btn-orange" style="padding: 0.3rem 0.75rem; font-size: 0.75rem;" onclick="returnCar(${r.rentalId})">
                        Return Car
                    </button>
                </td>
            </tr>
        `).join('');
    }

    function renderHistory() {
        if (rentals.length === 0) {
            historyTbody.innerHTML = '<tr><td colspan="6" class="text-center text-muted" style="padding: 1.5rem;">No transaction history yet.</td></tr>';
            return;
        }

        historyTbody.innerHTML = rentals.map(r => `
            <tr>
                <td class="font-mono">#${r.rentalId}</td>
                <td>${escapeHtml(r.carName)} (#${r.carId})</td>
                <td>${escapeHtml(r.customer)}</td>
                <td>${r.days} days</td>
                <td class="font-mono">₹${r.amount.toLocaleString('en-IN', { minimumFractionDigits: 2 })}</td>
                <td>
                    <span class="status-badge ${r.active ? 'status-rented' : 'status-available'}">
                        ${r.active ? 'Active' : 'Returned'}
                    </span>
                </td>
            </tr>
        `).join('');
    }

    function updateStats() {
        const total = fleet.length;
        const available = fleet.filter(c => c.available).length;
        const active = rentals.filter(r => r.active).length;
        const revenue = rentals.reduce((sum, r) => sum + r.amount, 0);

        statTotal.textContent = total;
        statAvailable.textContent = available;
        statRented.textContent = active;
        statRevenue.textContent = '₹' + revenue.toLocaleString('en-IN');
    }

    // ----------------- Booking Modal Flow -----------------
    window.openBookingModal = function(carId) {
        const car = fleet.find(c => c.id === carId);
        if (!car || !car.available) {
            showToast('This car is not available for booking.', 'error');
            return;
        }

        selectedCarForBooking = car;
        bookCarIdInput.value = car.id;
        modalCarName.textContent = car.name;
        modalCarBadge.textContent = car.type;
        modalCarBadge.className = `badge ${car.type === 'SUV' ? 'badge-suv' : 'badge-sedan'}`;
        modalCarRate.textContent = `₹${car.rate.toLocaleString('en-IN')}`;
        bookDaysInput.value = 1;
        bookCustomerInput.value = '';

        updateLiveBillCalculation();
        bookingModal.classList.add('active');
        bookCustomerInput.focus();
    };

    function closeBookingModal() {
        bookingModal.classList.remove('active');
        selectedCarForBooking = null;
    }

    function updateLiveBillCalculation() {
        if (!selectedCarForBooking) return;

        const days = Math.max(1, parseInt(bookDaysInput.value) || 1);
        const rate = selectedCarForBooking.rate;
        const base = days * rate;
        const isSUV = selectedCarForBooking.type === 'SUV';
        const surcharge = isSUV ? 300.0 : 0.0;
        const total = base + surcharge;

        calcDays.textContent = days;
        calcRate.textContent = `₹${rate}`;
        calcBase.textContent = `₹${base.toFixed(2)}`;
        suvSurchargeRow.style.display = isSUV ? 'flex' : 'none';
        calcTotal.textContent = `₹${total.toFixed(2)}`;
    }

    bookDaysInput.addEventListener('input', updateLiveBillCalculation);

    modalCloseBtn.addEventListener('click', closeBookingModal);
    modalCancelBtn.addEventListener('click', closeBookingModal);

    bookForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        if (!selectedCarForBooking) return;

        const carId = parseInt(bookCarIdInput.value);
        const customer = bookCustomerInput.value.trim();
        const days = parseInt(bookDaysInput.value);

        if (!customer) {
            showToast('Please enter a valid customer name.', 'error');
            return;
        }

        try {
            const res = await fetch('/api/book', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ carId, customer, days })
            });

            const data = await res.json();
            if (!res.ok || !data.success) {
                throw new Error(data.message || 'Booking failed');
            }

            showToast(`Success! Rental #${data.rentalId} confirmed for ${data.carName}. Total: ₹${data.amount.toFixed(2)}`, 'success');
            closeBookingModal();
            await fetchFleet();
            await fetchRentals();
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    // ----------------- Return Car Action -----------------
    window.returnCar = async function(rentalId) {
        if (!confirm(`Confirm returning vehicle for Rental #${rentalId}?`)) return;

        try {
            const res = await fetch('/api/return', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ rentalId })
            });

            const data = await res.json();
            if (!res.ok || !data.success) {
                throw new Error(data.message || 'Return failed');
            }

            showToast(`Vehicle returned successfully! Car ID #${data.carId} is now available in fleet.`, 'success');
            await fetchFleet();
            await fetchRentals();
        } catch (err) {
            showToast(err.message, 'error');
        }
    };

    // ----------------- Add Car Flow -----------------
    addCarForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = parseInt(document.getElementById('car-id').value);
        const name = document.getElementById('car-name').value.trim();
        const type = document.getElementById('car-type').value;
        const rate = parseFloat(document.getElementById('car-rate').value);

        if (!id || !name || !rate) {
            showToast('Please fill all car details correctly.', 'error');
            return;
        }

        try {
            const res = await fetch('/api/cars', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ id, name, type, rate })
            });

            const data = await res.json();
            if (!res.ok || !data.success) {
                throw new Error(data.message || 'Could not add car');
            }

            showToast(`Vehicle #${id} (${name}) added to C++ fleet!`, 'success');
            addCarForm.reset();
            await fetchFleet();
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    // Filter Buttons
    filterButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            filterButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            activeFilter = btn.dataset.filter;
            renderFleet();
        });
    });

    refreshRentalsBtn.addEventListener('click', () => {
        fetchRentals();
        fetchFleet();
        showToast('Fleet and rental status refreshed from C++ engine.', 'success');
    });

    function escapeHtml(str) {
        if (!str) return '';
        return String(str)
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;')
            .replace(/'/g, '&#039;');
    }

    // Initial Load
    fetchFleet();
    fetchRentals();
});
